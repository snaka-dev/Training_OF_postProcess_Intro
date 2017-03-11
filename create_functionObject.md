# OpenFOAMのポスト処理自動化 入門    #

##### オープンCAE勉強会＠富山　March 5, 2017，　富山県立大学　中川慎二

## まえがき

Disclaimer: OPENFOAM is a registered trade mark of OpenCFD Limited, the producer of the OpenFOAM software and owner of the OPENFOAM and OpenCFD trade marks. This offering is not approved or endorsed by OpenCFD Limited.

OpenFOAMユーザーガイド，プログラマーズガイド，OpenFOAM Wiki，CFD Online，その他多くの情報を参考にしています。開発者，情報発信者の皆様に深い謝意を表します。

この講習内容は，講師の個人的な経験（主に，卒研生等とのコードリーディング）から得た知識を共有するものです。この内容の正確性を保証することはできません。この情報を使用したことによって問題が生じた場合，その責任は負いかねますので，予めご了承ください。

<a name="notation"></a>
## 本文書での表記方法について

#### 端末から入力するコマンド

端末（ターミナル）で実行するコマンドは，次のように表記する。

> cp a b

#### ファイルやソースコードの内容

ファイル・ソースコード記載事項は次のように表記する。インデント（字下げ）は，必ずしもここに記載通りとは限らない。記入するファイルに合わせて，適切にインデントしてください。

```
solve
(
    fvm::ddt(T)
  - fvm::laplacian(DT, T)
);
```

## 目的 ##

今回の講習の目的は，OpenFOAMの計算実行時およびポスト処理時において，計算結果をチェックするための作業を自動化する方法の基礎を身につけることである。

ポスト処理に用いられるOpenFOAMの機能である functionObject 機能に注目し，新たな functionObject を作成する方法を学びます。

密閉容器内の自然対流を対象とした解析(buoyantCavity例題)を実施します。計算対象の熱収支をチェックできるように，OpenFOAMの基本的なプログラミングに挑戦して，自動処理を可能とします。具体的には，wallHeatFluxユーティリティを参考にしながら，固体面から流入出する熱量の時間履歴をファイルに出力するfunctionObjectを作成し，実行時にオンタイムでグラフ表示することを目指します。

添付図のように，各面からの流入出熱量，それらを合計した熱収支を，計算実行時にオンタイムでグラフ化します。

| <img src=./img/wallHeatBalance-graph.png alt="熱収支のグラフ" title="熱収支のグラフ" width=600px> |
|:-------:|
|  図 　熱収支のグラフ  |

## Note ##

Training materials for mini lecture course for OpenCAE Study Group @ Toyama.

http://eddy.pu-toyama.ac.jp/%E3%82%AA%E3%83%BC%E3%83%97%E3%83%B3CAE%E5%8B%89%E5%BC%B7%E4%BC%9A-%E5%AF%8C%E5%B1%B1/

https://www.facebook.com/OpenCAEstudyGroupAtToyama


## 環境 ##

この資料は，OpenFOAM 4.1 を基準として作成した。OpenFOAM 4から，ポスト処理のコードが大幅に改編されている。



## 想定する受講者・前提知識 ##

- OpenFOAMのごく基本的な事を知っている。（例題の1つや2つを実行したことがある。）

- 何らかのプログラミング言語を学習したことがあり，プログラミングに関するごく基本的な知識がある。（変数，関数，型，などの基礎知識）

- Linuxの端末上で，ごく基本的な操作ができる。（テキストに書いてあることをタイプして実行できる。）

- Linux上で，ファイルのコピーや移動ができる。　

- 過去の講習（20160820第46回オープンCAE勉強会＠富山，OpenFOAMユーザーのためのシェルスクリプト入門 by 中山勝之）資料を見て，OpenFOAM実行スクリプトAllrunの内容を理解している。
    - 　[OpenFOAMユーザーのためのシェルスクリプト入門.pdf](http://eddy.pu-toyama.ac.jp/%E3%82%AA%E3%83%BC%E3%83%97%E3%83%B3CAE%E5%8B%89%E5%BC%B7%E4%BC%9A-%E5%AF%8C%E5%B1%B1/?action=cabinet_action_main_download&block_id=99&room_id=1&cabinet_id=1&file_id=154&upload_id=288 "OpenFOAMユーザーのためのシェルスクリプト入門.pdf")


- 過去の講習（20170121第51回オープンCAE勉強会＠富山，OpenFOAMのポスト処理自動化 超入門）資料を見て，OpenFOAMのポスト処理の基本を理解している。
    - 　[OpenFOAMのポスト処理自動化 超入門テキスト：introduction.md](https://github.com/snaka-dev/Training_OF_postProcess_Intro)



<a name="tableOfContents"></a>
## 目次 ##

1.   [本文書での表記方法について            ](#notation)
2.   [準備                             ](#pre)
  1. [Linuxコマンドの確認                  ](#linuxCommand)
  2. [環境変数の確認                     ](#checkEnvVariables)
3.   [講習の流れ                         ](#section1)
4.   [例題のコピーと設定変更と実行            ](#section2)
5.   [wallHeatFluxユーティリティのコード確認                  ](#section3)
6.   [wallShearStress のコード確認              ](#section4)
7.   [foamNewFunctionObject ユーティリティ ](#section5)
8.   [wallHeatBalanceの作成                             ](#section6)
  1. [wallHeatBalance.H                  ](#section6-1)
  2. [wallHeatBalance.C                   ](#section6-2)
  3. [Makeフォルダ                   ](#section6-3)


<a name="pre"></a>
## 準備

<a name="linuxCommand"></a>
### Linuxコマンドの確認

端末内での実行場所移動：cd （チェンジ ディレクトリ）

> cd _移動先_

ディレクトリの作成：mkdir （メーク ディレクトリ）

> mkdir _ディレクトリ名_

　オプション　-p   親ディレクトリも同時に作成

ファイルやディレクトリのコピー：cp （コピー）

> cp _元ファイル_ _コピー先_

オプション　-p   元のファイル属性を保持（preserve）
オプション　-r   ディレクトリの中身もコピー ← 再帰的にコピー（recursive）

ファイルやディレクトリの移動：mv （ムーブ）

> mv _移動元_ _移動先_

　このコマンドは，名前の変更にも使う。


<a name="checkEnvVariables"></a>
### 環境変数の確認

環境変数の確認方法など。各自の環境変数を調べ，別紙に記入してください。

システムの例題格納場所：$FOAM_TUTORIALS
> echo $FOAM_TUTORIALS

ユーザーのOpenFOAM作業場所：$FOAM_RUN
> echo $FOAM_RUN


[［手順一覧に戻る］](#tableOfContents)


<a name="section1"></a>
## 講習の流れ ##

buoyantSimpleFoam/buoyantCavity 例題を元にする。

初期例題をそのまま実行　→　wallHeatFlux ユーティリティを実行し，熱量バランスを見る。

wallHeatFlux ユーティリティのコードを確認する。

壁面での値を取り扱う既存の functionObject である wallShearStress のコードを確認する。

foamNewFunctionObject ユーティリティを実行し，functionObject のひな形を作成する。

熱量バランスの時系列データを出力する functionObject として，wallHeatBalance を作成する。

[手順一覧に戻る](#tableOfContents)


<a name="section2"></a>
## 例題のコピーと設定変更と実行 ##

### 例題のコピー ###

作業ディレクトリ（$FOAM_RUN）に例題（$FOAM_TUTORIALS/heatTransfer/buoyantSimpleFoam/buoyantCavity/）をコピーする。コピーして作成したケースの名前は buoyantCavityTest とする。

> mkdir -p $FOAM_RUN

> cp -r $FOAM_TUTORIALS/heatTransfer/buoyantSimpleFoam/buoyantCavity/ $FOAM_RUN/buoyantCavityTest


### fvSolutionの変更 ###
system/fvSolution ファイルを下記の様に変更する。

48行目付近　収束判定基準を少し弱める。62行，67行の緩和係数を大きくする。収束を早める。

```
residualControl
{
    p_rgh           3e-4; //1e-4;
    U               3e-4; //1e-4;
    h               3e-4; //1e-4;
```

```
relaxationFactors
{
    fields
    {
        rho             1.0;
        p_rgh           0.95; //0.7;
        equations
        {
            U               0.3;
            h               0.95; //0.3;
```

### 残差出力のためにsystem/residualsファイルを作成 ###

例題のsystemディレクトリに，残差出力設定用ファイル residuals を作成する。作成したresidualsファイルに下記を書き込む。

```
residuals
{
    type            residuals;
    functionObjectLibs ("libutilityFunctionObjects.so");

    writeControl   timeStep;
    writeInterval  50;

    fields ( h k omega p_rgh); // U

}
```

### 残差出力のためにsystem/controlDictファイルを修正 ###

作成したresidualファイルの内容を実行時に参照するように，system/controlDictファイルを修正する。controlDictファイルの下部に下記を追加する。

```
functions
{
    #include "residuals"
}
```

### 計算実行 ###

端末から例題ディレクトリに移動し，計算実行スクリプト Allrun を実行する。

> cd $FOAM_RUN/buoyantCavityTest
>
> ./Allrun

実行が継続している間に，別の端末を起動する。（計算実行中の端末で右クリック，Open Terminalを選択してもよい。）

端末から下記コマンドを実行し，残差ファイルをグラフ表示する。

> foamMonitor -l postProcessing/﻿residuals/0/residuals.dat

表示されるグラフを下図に示す。

| <img src=./img/residualHistory.png alt="残差履歴のグラフ" title="残差履歴のグラフ" width=600px> |
|:-------:|
|  図 　残差履歴のグラフ  |


### ポスト処理出力の確認：sample ###

ディレクトリ postProcessing/ に sample ディレクトリが作成されていることを確認する。その下にある 869ディレクトリの中身を確認する。これらのファイルは，Allrunスクリプトで実行した"postProcess -latestTime -func sample"により作成されたものである。どのような情報を取り出したものかは，system/sampleファイルを確認する。

### ポスト処理出力の確認：residuals ###

ディレクトリ postProcessing/ に　residuals ディレクトリが作成されていることを確認する。その下にある 0/residuals.dat ファイルを開いて内部を確認する。

### 計算結果の確認：ParaFoam ###

### 計算結果の確認：wallHeatFlux ###

加熱面から流入する熱量と，冷却面から流出する熱量のバランスを確認するため，wallHeatFluxユーティリティを実行する。

> wallHeatFlux

実行結果から，加熱面（patch名 hot）から対流によって36.078 W，冷却面（patch名 cold）から -36.0998 Wが流入していることを確認する。

wallHeatFluxユーティリティでは，保存された結果に対して各面での熱量を知ることができる。しかし，計算実行中にそれらの値を知ることや，時間変化をグラフ化しやすい形のデータファイルとして保存することはできない。

[手順一覧に戻る](#tableOfContents)


<a name="section3"></a>
## wallHeatFluxユーティリティのコード確認 ##

OpenFOAM-4.x/applications/utilities/postProcessing/toBeFunctionObjects/wallHeatFlux/

https://github.com/OpenFOAM/OpenFOAM-4.x/tree/be7fba6cff9bbf6820c5cb278a3ecad6bea61241/applications/utilities/postProcessing/toBeFunctionObjects/wallHeatFlux

https://cpp.openfoam.org/v4/dir_96c0dce7250601ccd0e50dd58c2d01d1.html

この wallHeatFlux ユーティリティは，時期バージョンから functionObject に変更されることが決まっている。（OpenFOAM-devでは，functionObjectである。）

wallHeatFlux.C において，面での熱流束を求めるコードは下記となる。

``` C++
        surfaceScalarField heatFlux
        (
            fvc::interpolate
            (
                (
                    turbulence.valid()
                  ? turbulence->alphaEff()()
                  : thermo->alpha()
                )
            )*fvc::snGrad(h)
        );
```

エネルギ勾配 snGrad(h)［単位：J/kg/m］ と 温度拡散率[kg/m/s]をかけることで，面での熱流束 heatFlux [J/m/m/s = W/m/m]を求めている．乱流計算の場合には，層流のalphaと乱流による温度拡散係数alphatとの和である有効温度拡散係数alphaEffが使用される。層流計算では，流体物性の温度拡散係数が使用される。

エネルギ h は，createFields.H において，`const volScalarField& h = thermo->he();`と定義されている。さらに検討していくと，basicThermo.H で定義されている Enthalpy または Internal energy [J/kg] であることがわかる。


bashicThermo.Hでの定義

```
//- Enthalpy/Internal energy [J/kg]
//  Non-const access allowed for transport equations
            virtual volScalarField& he() = 0;
```

/opt/openfoam4/src/TurbulenceModels/compressible/EddyDiffusivity/

```
        //- Return the effective turbulent thermal diffusivity for enthalpy
        //  [kg/m/s]
        virtual tmp<volScalarField> alphaEff() const
        {
            return this->transport_.alphaEff(alphat());
        }
```

/opt/openfoam4/src/TurvbulenceModels/compressible/ThermalDiffusivity.H

```
        //- Return the effective turbulent thermal diffusivity for enthalpy
        //  [kg/m/s]
        virtual tmp<volScalarField> alphaEff() const
        {
            return alpha();
        }

        //- Return the laminar thermal diffusivity for enthalpy [kg/m/s]
        virtual tmp<volScalarField> alpha() const
        {
            return this->transport_.alpha();
        }
```

[手順一覧に戻る](#tableOfContents)


<a name="section4"></a>
## wallShearStress のコード確認 ##

/opt/openfoam4/src/functionObjects/field/wallShearStress/


[手順一覧に戻る](#tableOfContents)


<a name="section5"></a>
## foamNewFunctionObject ユーティリティ  ##

foamNewFunctionObject を実行する。必要な引数の無い状態で実行し，その使い方を表示させると，次のようになる。

> $ foamNewFunctionObject
Wrong number of arguments
Usage: foamNewFunctionObject [-h | -help] <functionObjectName>
>
* Create directory with source and compilation files for a new function object
  <functionObjectName> (dir)
  - <functionObjectName>.H
  - <functionObjectName>.C
  - IO<functionObjectName>.H
  - Make (dir)
    - files
    - options
  Compiles a library named lib<functionObjectName>FunctionObject.so in
  $FOAM_USER_LIBBIN:
  /home/user/OpenFOAM/user-4.x/platforms/linux64GccDPInt32Opt/lib

wallHeatBalance という名前のfunctionObjectを作成することにする。よって，下記のコマンドを実行する。

> $ foamNewFunctionObject wallHeatBalance

ここで作成されたひな形のクラスは， fvMeshFunctionObject を継承している。このfvMeshFunctionObject クラスは，regionFunctionObject クラスを継承している。（/src/finiteVolume/fvMesh/fvMeshFunctionObject/fvMeshFunctionObject.H）

wallHeatBalance → fvMeshFunctionObject → regionFunctionObject

wallHeatBalance.H を見る。主要な要素は，下記の4つの関数である。

```
        //- Read the wallHeatBalance data
        virtual bool read(const dictionary&);

        //- Execute, currently does nothing
        virtual bool execute();

        //- Execute at the final time-loop, currently does nothing
        virtual bool end();

        //- Write the wallHeatBalance
        virtual bool write();
```

これらの少しだけ詳しい説明は，下記のファイルにある。

/opt/openfoam4/src/OpenFOAM/db/functionObjects/functionObject/functionObject.H

```

        //- Read and set the function object if its data have changed
        virtual bool read(const dictionary&);

        //- Called at each ++ or += of the time-loop.
        //  postProcess overrides the usual executeControl behaviour and
        //  forces execution (used in post-processing mode)
        virtual bool execute() = 0;

        //- Called at each ++ or += of the time-loop.
        //  postProcess overrides the usual writeControl behaviour and
        //  forces writing always (used in post-processing mode)
        virtual bool write() = 0;

        //- Called when Time::run() determines that the time-loop exits.
        //  By default it simply calls execute().
        virtual bool end();

```

[手順一覧に戻る](#tableOfContents)


<a name="section6"></a>
## wallHeatBalanceの作成 ##

具体的にコードの作成をはじめる。今回作成するコードの目的は次の通りである。

+ 固体面から流入出する熱量の時間履歴をファイルに出力する

+ 実行時にオンタイムでグラフ表示できる形式のファイルとする（foamMonitorで表示する）

ファイルへの書き出しや読み込みは，wallShearStressを参考にする．熱量算出部分は，wallHeatFluxユーティリティを参考にする．

先述のとおり，wallHeatFluxユーティリティは，時期OpenFOAMからは，functionObjectになる．OpenFOAM-devのgitリポジトリには，functionObject となったwallHeatFluxコードが存在する。これも参考にする。OpenFOAM 4とOpenFOAM-devとでは，コードの細部に違いがあるため，そのままコピペしてもコンパイルできない。（エラーメッセージを見ながら修正することはできる。）

https://github.com/OpenFOAM/OpenFOAM-dev/tree/master/src/functionObjects/field/wallHeatFlux


[手順一覧に戻る](#tableOfContents)


<a name="section6-1"></a>
## wallHeatBalance.H のコード ##

クラス名は， wallHeatBalance であり，writeFilesクラス（ functionObject base class for writing files）を継承することとする。これは，wallShearStressクラスと同様に，ファイルの入出力を可能にする可能性があるためである。

先に示したひな形では，fvMeshFunctionObject を継承することとなっている。writeFiles クラスは，fvMeshFunctionObject を継承しているため，両者を継承すると良くない。writeFilesクラスの継承関係は次の通りである。

writeFiles → writeFile → regionFunctionObject → functionObject 

    
wallHeatBalance.H のコードは次のようになる。

```

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
```



<a name="section6-2"></a>
## wallHeatBalance.C のコード ##



wallHeatBalance.C は次のように。

```
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
```


[手順一覧に戻る](#tableOfContents)


<a name="section6-3"></a>
## Makeフォルダ ##

### files

```
wallHeatBalance.C

LIB = $(FOAM_USER_LIBBIN)/libwallHeatBalanceFunctionObject

```

### options

```
EXE_INC = \
    -I$(LIB_SRC)/finiteVolume/lnInclude \
    -I$(LIB_SRC)/meshTools/lnInclude \
    -I$(LIB_SRC)/thermophysicalModels/basic/lnInclude \
    -I$(LIB_SRC)/transportModels \
    -I$(LIB_SRC)/transportModels/compressible/lnInclude \
    -I$(LIB_SRC)/TurbulenceModels/turbulenceModels/lnInclude \
    -I$(LIB_SRC)/TurbulenceModels/incompressible/lnInclude \
    -I$(LIB_SRC)/TurbulenceModels/compressible/lnInclude

LIB_LIBS = \
    -lfiniteVolume \
    -lfluidThermophysicalModels \
    -lincompressibleTransportModels \
    -lturbulenceModels \
    -lcompressibleTransportModels \
    -lincompressibleTurbulenceModels \
    -lcompressibleTurbulenceModels \
    -lmeshTools 

```

[手順一覧に戻る](#tableOfContents)

