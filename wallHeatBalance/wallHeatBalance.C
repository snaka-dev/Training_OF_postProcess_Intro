/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2016 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "wallHeatBalance.H"

#include "volFields.H"
#include "surfaceFields.H"
#include "turbulentTransportModel.H"
#include "turbulentFluidThermoModel.H"
#include "wallPolyPatch.H"

#include "fvc.H"

#include "addToRunTimeSelectionTable.H"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(wallHeatBalance, 0);
    addToRunTimeSelectionTable(functionObject, wallHeatBalance, dictionary);
}
}

// * * * * * * * * * * * * * Protected Member Functions * * * * * * * * * * //

void Foam::functionObjects::wallHeatBalance::writeFileHeader(const label i)
{

    // Add headers to output data
    writeHeader(file(), "Wall heat-flux");
    writeCommented(file(), "Time");


    const fvMesh& mesh = refCast<const fvMesh>(obr_);
    const fvPatchList& patches = mesh.boundary();

    forAllConstIter(labelHashSet, patchSet_, iter)
    {
        label patchi = iter.key();
        const fvPatch& pp = patches[patchi];
        writeTabbed(file(), pp.name() );
    }

    writeTabbed(file(), "Balance");
    file() << endl;

}

void Foam::functionObjects::wallHeatBalance::calcWallHeatFlux
(
    const compressible::turbulenceModel& turbulence,
    volScalarField& wallHeatFlux
)
{
    const fvMesh& mesh = refCast<const fvMesh>(obr_);

    const volScalarField& h = turbulence.transport().he();

    surfaceScalarField heatFlux
    (
        fvc::interpolate
        (
            (
                turbulence.alphaEff()
            )
        )*fvc::snGrad(h)
    );


    volScalarField::Boundary& wallHeatFluxBf =
       wallHeatFlux.boundaryFieldRef();


    const surfaceScalarField::Boundary& patchHeatFlux =
        heatFlux.boundaryField();


    forAll(wallHeatFluxBf, patchi)
    {
        wallHeatFluxBf[patchi] = patchHeatFlux[patchi];
    }

    if (mesh.foundObject<volScalarField>("Qr"))
    {
        const volScalarField& Qr = mesh.lookupObject<volScalarField>("Qr");

        const volScalarField::Boundary& radHeatFluxBf =
            Qr.boundaryField();

        forAll(wallHeatFluxBf, patchi)
        {
            wallHeatFluxBf[patchi] += radHeatFluxBf[patchi];
        }

    }

}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::wallHeatBalance::wallHeatBalance
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    writeFiles(name, runTime, dict, name),
    patchSet_()
{
    if (!isA<fvMesh>(obr_))
    {
        FatalErrorInFunction
            << "objectRegistry is not an fvMesh" << exit(FatalError);
    }

    const fvMesh& mesh = refCast<const fvMesh>(obr_);

    volScalarField* wallHeatBalancePtr
    (
        new volScalarField
        (
            IOobject
            (
                type(),
                mesh.time().timeName(),
                mesh,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh,
            dimensionedScalar("0", dimMass/pow3(dimTime), 0)
        )
    );

    mesh.objectRegistry::store(wallHeatBalancePtr );

    read(dict);
    resetName(typeName);

}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::wallHeatBalance::~wallHeatBalance()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::wallHeatBalance::read(const dictionary& dict)
{
    // from OpenFOAM-4.x/src/functionObjects/field/wallShearStress/wallShearStress.C
    writeFiles::read(dict);

    const fvMesh& mesh = refCast<const fvMesh>(obr_);
    const polyBoundaryMesh& pbm = mesh.boundaryMesh();

    patchSet_ =
        mesh.boundaryMesh().patchSet
        (
            wordReList(dict.lookupOrDefault("patches", wordReList()))
        );

    Info<< type() << " " << name() << ":" << nl;

    if (patchSet_.empty())
    {
        forAll(pbm, patchi)
        {
            if (isA<wallPolyPatch>(pbm[patchi]))
            {
                patchSet_.insert(patchi);
            }
        }

        Info<< " processing all wall patches" << nl << endl;
    }
    else
    {
        Info<< " processing wall patches: " << nl;
        labelHashSet filteredPatchSet;
        forAllConstIter(labelHashSet, patchSet_, iter)
        {
            label patchi = iter.key();
            if (isA<wallPolyPatch>(pbm[patchi]))
            {
                filteredPatchSet.insert(patchi);
                Info<< " " << pbm[patchi].name() << endl;
            }
            else
            {
                WarningInFunction
                    << "Requested wall shear stress on non-wall boundary "
                    << "type patch: " << pbm[patchi].name() << endl;
            }
        }

        Info<< endl;

        patchSet_ = filteredPatchSet;
    }

      return true;
}


    bool Foam::functionObjects::wallHeatBalance::execute()
{
    volScalarField& wallHeatFlux = const_cast<volScalarField&>
    (
        lookupObject<volScalarField>(type())
    );

    if
    (
        foundObject<compressible::turbulenceModel>
        (
            turbulenceModel::propertiesName
        )
    )
    {
        const compressible::turbulenceModel& turbModel =
            lookupObject<compressible::turbulenceModel>
            (
                turbulenceModel::propertiesName
            );

        calcWallHeatFlux(turbModel, wallHeatFlux);
    }
    else
    {
        FatalErrorInFunction
            << "Unable to find compressible turbulence model in the "
            << "database" << exit(FatalError);
    }

    return true;
}


bool Foam::functionObjects::wallHeatBalance::end()
{
    return true;
}


bool Foam::functionObjects::wallHeatBalance::write()
{
    // set directory and file names
    //   src/OpenFOAM/db/functionObjects/writeFiles
    writeFiles::write();

    const volScalarField& wallHeatFlux =
        obr_.lookupObject<volScalarField>(type());

    //Log << type() << " " << name() << " write:" << nl;
    //    << "    writing field " << wallheatFlux.name() << endl;
    //wallHeatFlux.write();

    Log << type() << endl;


    const fvMesh& mesh = refCast<const fvMesh>(obr_);
    const fvPatchList& patches = mesh.boundary();

    const surfaceScalarField::Boundary& magSf =
        mesh.magSf().boundaryField();

    scalar totalIntegralHfp = 0.0;

    if (Pstream::master())
    {
        file()
            << mesh.time().value() ;
    }

    forAllConstIter(labelHashSet, patchSet_, iter)
    {
        label patchi = iter.key();
        const fvPatch& pp = patches[patchi];

        const scalarField& hfp = wallHeatFlux.boundaryField()[patchi];

        const scalar minHfp = gMin(hfp);
        const scalar maxHfp = gMax(hfp);
        const scalar integralHfp = gSum(magSf[patchi]*hfp);

        totalIntegralHfp += integralHfp;

        if (Pstream::master())
        {
            file()
                //<< mesh.time().value()
                //<< token::TAB << pp.name()
                //<< token::TAB << minHfp
                //<< token::TAB << maxHfp
                << token::TAB << integralHfp
                //<< token::TAB
                //<< endl
                ;
        }

        Log << "    min/max/integ(" << pp.name() << ") = "
            << minHfp << ", " << maxHfp << ", " << integralHfp
            << endl;
    }

    if (Pstream::master())
    {
        file()
            << token::TAB << totalIntegralHfp
            << endl;
    }

    Log << endl ;

    return true;
}


// ************************************************************************* //

