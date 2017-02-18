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

Class
    Foam::functionObjects::wallHeatBalance

Group

Description
    This function object evaluates and outputs the incoming heat [W] 
    at wall patches.  The result is written as a time-history data 
    to wallHeatBalance-patchName.
   

    Example of function object specification:
    \verbatim
    wallHeatBalance1
    {
        type           wallHeatBalance;
        libs ("libwallHeatBalanceFunctionObject.so");
        patches ();

    }
    \endverbatim

Usage
    \table
        Property    | Description                | Required | Default value
        type        | type name: wallHeatBalance | yes      |
        patches     | list of patches to process | no       | all wall patches

    \endtable

SourceFiles
    wallHeatBalance.C

\*---------------------------------------------------------------------------*/

#ifndef wallHeatBalance_H
#define wallHeatBalance_H

#include "writeFiles.H"
#include "volFieldsFwd.H"
#include "turbulentFluidThermoModel.H"
#include "HashSet.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

namespace functionObjects
{

/*---------------------------------------------------------------------------*\
                   Class wallHeatBalance Declaration
\*---------------------------------------------------------------------------*/

class wallHeatBalance
:
    // functionObject base class for writing files
    //    src/OpenFOAM/db/functionObjects/writeFiles
    public writeFiles
{
protected:

    // Protected data

        //- Optional list of patches to process
        labelHashSet patchSet_;

    // Protected Member Functions

        //- File header information
        virtual void writeFileHeader(const label i);

        //- Calculate the wall Heat Flux
        void calcWallHeatFlux
        (
            const compressible::turbulenceModel& turbulence,
            volScalarField& wallHeatFlux
        );

    // Private Member Functions

        //- Disallow default bitwise copy construct
        wallHeatBalance(const wallHeatBalance&);

        //- Disallow default bitwise assignment
        void operator=(const wallHeatBalance&);


public:

    //- Runtime type information
    TypeName("wallHeatBalance");


    // Constructors

        //- Construct from Time and dictionary
        wallHeatBalance
        (
            const word& name,
            const Time& runTime,
            const dictionary& dict
        );


    //- Destructor
    virtual ~wallHeatBalance();


    // Member Functions

        //- Read the wallHeatBalance data
        virtual bool read(const dictionary&);

        //- Execute, currently does nothing
        virtual bool execute();

        //- Execute at the final time-loop, currently does nothing
        virtual bool end();

        //- Write the wallHeatBalance
        virtual bool write();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace functionObjects
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
