# OpenFOAMのポスト処理自動化 超入門 #

##### オープンCAE勉強会＠富山　June 21, 2017，　富山県立大学　中川慎二

## まえがき

Disclaimer: OPENFOAM® is a registered trade mark of OpenCFD Limited, the producer of the OpenFOAM software and owner of the OPENFOAM ® and OpenCFD ® trade marks. This offering is not approved or endorsed by OpenCFD Limited.

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

## Note ##

Training materials for mini lecture course for OpenCAE Study Group @ Toyama.
https://www.facebook.com/OpenCAEstudyGroupAtToyama

http://eddy.pu-toyama.ac.jp/%E3%82%AA%E3%83%BC%E3%83%97%E3%83%B3CAE%E5%8B%89%E5%BC%B7%E4%BC%9A-%E5%AF%8C%E5%B1%B1/

## 環境 ##

この資料は，OpenFOAM 4.1 を基準として作成した。OpenFOAM 4から，ポスト処理のコードが大幅に改編されている。OpenFOAM3については，後半で検討する。

## 想定する受講者・前提知識 ##

- OpenFOAMのごく基本的な事を知っている。（例題の1つや2つを実行したことがある。）

- 何らかのプログラミング言語を学習したことがあり，プログラミングに関するごく基本的な知識がある。（変数，関数，型，などの基礎知識）

- Linuxの端末上で，ごく基本的な操作ができる。（テキストに書いてあることをタイプして実行できる。）

- Linux上で，ファイルのコピーや移動ができる。　

- 過去の講習（20160820第46回オープンCAE勉強会＠富山，OpenFOAMユーザーのためのシェルスクリプト入門 by 中山勝之）資料を見て，OpenFOAM実行スクリプトAllrunの内容を理解している。　[OpenFOAMユーザーのためのシェルスクリプト入門.pdf](http://eddy.pu-toyama.ac.jp/%E3%82%AA%E3%83%BC%E3%83%97%E3%83%B3CAE%E5%8B%89%E5%BC%B7%E4%BC%9A-%E5%AF%8C%E5%B1%B1/?action=cabinet_action_main_download&block_id=99&room_id=1&cabinet_id=1&file_id=154&upload_id=288 "OpenFOAMユーザーのためのシェルスクリプト入門.pdf")


<a name="tableOfContents"></a>
## 目次 ##

1.   [本文書での表記方法について            ](#notation)
2.   [準備                             ](#pre)
  1. [Linuxコマンドの確認                  ](#linuxCommand)
  2. [環境変数の確認                     ](#checkEnvVariables)
3.   [講習の流れ                         ](#section1)
4.   [例題のコピーと設定変更と実行            ](#section2)
5.   [指定点での履歴確認                  ](#section3)
6.   [実行時可視可画像の作成              ](#section4)
7.   [領域内の平均値，最小値，最大値を求める ](#section5)
8.   [memo                             ](#section6)
8.   [For OpenFOAM 3                   ](#section7)


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

初期例題をそのまま実行　→　欲しい情報を決める

一つずつ，機能を追加する。

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


#### OpenFOAM 3.0.x で残差グラフがうまく表示されない場合 ####

foamMonitor を実行したとき，「foamMonitor: [[: not found」というエラーメッセージが表示され，グラフが自動更新されない場合がある。その場合，foamMonitor スクリプトの158行目の次のように修正する。

修正前　if [[ $i -lt $NCOLS ]]

修正後　if [ $i -lt $NCOLS ]

### ポスト処理出力の確認：sample ###

ディレクトリ postProcessing/ に sample ディレクトリが作成されていることを確認する。その下にある 869ディレクトリの中身を確認する。これらのファイルは，Allrunスクリプトで事項した"postProcess -latestTime -func sample"により作成されたものである。どのような情報を取り出したものかは，system/sampleファイルを確認する。

### ポスト処理出力の確認：residuals ###

ディレクトリ postProcessing/ に　residuals ディレクトリが作成されていることを確認する。その下にある 0/residuals.dat ファイルを開いて内部を確認する。

### 計算結果の確認：ParaFoam ###

### 計算結果の確認：wallHeatFlux ###

加熱面から流入する熱量と，冷却面から流出する熱量のバランスを確認するため，wallHeatFluxユーティリティを実行する。

> wallHeatFlux

実行結果から，加熱面（patch名 hot）から対流によって36.078 W，冷却面（patch名 cold）から -36.0998 Wが流入していることを確認する。

[手順一覧に戻る](#tableOfContents)


<a name="section3"></a>
## 指定点での履歴確認 ##

probes

probe で指定点のデータ


### 指定点での値出力のためにsystem/probesファイルを作成 ###

例題のsystemディレクトリに，モニタリング点設定用ファイル probes を作成する。作成したprobesファイルに下記を書き込む。

```
probes
{
    type probes;
    functionObjectLibs ("libsampling.so");

    enabeld true;
    writeControl timeStep;
    writeInterval 50;

    // Fields to be probed. runTime modifiable!
    fields
    (
        p_rgh T U k omega
    );

    // Locations to be probed. runTime modifiable!
    probeLocations
    (
        (0.038 1.0901 0.0) //center
        (0.038 0.654 0.0)  // y=0.3h

        (0.001 1.0901 0.0) // x=0.001, y=0.5h
        (0.010 1.0901 0.0) // x=0.010, y=0.5h
        (0.066 1.0901 0.0) // x=0.066, y=0.5h
        (0.075 1.0901 0.0) // x=0.075, y=0.5h

        (0.075 2.17 0.0) // x=0.075, y=2.18-0.01
        (0.001 0.01 0.0) // x=0.001, y=0.01

    );
}
```

### 指定点での値出力のためにsystem/controlDictファイルを修正 ###

作成したprobesファイルの内容を実行時に参照するように，system/controlDictファイルを修正する。controlDictファイルの下部に下記を追加する。

```
functions
{
    #include "residuals"
    #include "probes"
}
```

### 計算の実行 ###

下記のコマンドを実行して，前回の結果を削除する。

> ./Allclean

下記のコマンドを実行して，計算を実行する。

> ./Allrun

### ポスト処理出力の確認：probes ###

ディレクトリ postProcessing/ に probes/0 ディレクトリが作成されていることを確認する。その下にある各変数のファイルを開いて内部を確認する。

端末から下記コマンドを実行し，温度の時刻歴ファイルをグラフ表示する。

> foamMonitor postProcessing/﻿probes/0/T 

表示されるグラフを下図に示す。

| <img src=./img/probesTHistory.png alt="probesで抽出した温度履歴のグラフ" title="probesで抽出した温度履歴のグラフ" width=600px> |
|:-------:|
|  図 　probesで抽出した温度履歴のグラフ  |


[手順一覧に戻る](#tableOfContents)


<a name="section4"></a>
## 実行時可視可画像の作成 ##

sample で cuttingPlane を作成する。

sample で streamline を作成する。

Paraviewを使って，計算実行時に作成された画像ファイル（vtk）を描画する。

### カット面での等値線図作成のためにsystem/cuttingPlaneファイルを作成 ###

例題のsystemディレクトリに，残差出力設定用ファイル cuttingPlane を作成する。作成したcuttingPlaneファイルに下記を書き込む。

```
cuttingPlane
{
    type            surfaces;
    libs ("libsampling.so");
    writeControl    writeTime;

    surfaceFormat   vtk;
    fields          ( p p_rgh U T omega );

    interpolationScheme cellPoint;

    surfaces
    (
        zNormal
        {
            type            cuttingPlane;
            planeType       pointAndNormal;
            pointAndNormalDict
            {
                basePoint       (0 0 0);
                normalVector    (0 0 1);
            }
            interpolate     true;
        }
    );
}
```

### 流線図作成のためにsystem/streamLinesファイルを作成 ###

例題のsystemディレクトリに，残差出力設定用ファイル streamLines を作成する。作成したstreamLinesファイルに下記を書き込む。

```
streamLines
{
    // Where to load it from
    libs ("libfieldFunctionObjects.so");

    type            streamLine;

    // Output every
    writeControl    writeTime;
    // writeInterval 10;

    setFormat       vtk; //gnuplot; //xmgr; //raw; //jplot; //csv; //ensight;

    // Tracked forwards (+U) or backwards (-U)
    trackForward    true;

    // Names of fields to sample. Should contain above velocity field!
    fields (p k T U);

    // Steps particles can travel before being removed
    lifeTime        10000;

    //- Specify either absolute length of steps (trackLength) or a number
    //  of subcycling steps per cell (nSubCycle)

        // Size of single track segment [m]
        //trackLength 1e-3;

        // Number of steps per cell (estimate). Set to 1 to disable subcycling.
        nSubCycle 5;


    // Cloud name to use
    cloudName       particleTracks;

    // Seeding method.
    seedSampleSet   uniform;  //cloud; //triSurfaceMeshPointSet;

    uniformCoeffs
    {
        type        uniform;
        axis        x;  //distance;

        // Note: tracks slightly offset so as not to be on a face
        start       (0.0001 1.0901 0.0001);
        end         (0.0759 1.0901 0.0001);
        nPoints     20;
    }
}
```

### カット面および流線図作成のためにsystem/controlDictファイルを修正 ###

作成したcuttingPlaneファイルおよびstreamLinesファイルの内容を実行時に参照するように，system/controlDictファイルを修正する。controlDictファイルの下部に下記を追加する。

```
functions
{
    #include "residuals"
    #include "probes"
    #include "cuttingPlane"
    #include "streamLines"
}
```

### ポスト処理出力の確認：cuttingPlane ###

ディレクトリ postProcessing/ に cuttingPlane ディレクトリが作成されていることを確認する。その下にある各時刻ディレクトリの中にあるファイルを開いて内部を確認する。

これらのファイルは，vtk形式のファイルである。このままでは，画像として表示されない。ParaViewから読み込むと，画像として表示できる。

このvtkファイルから画像を作成する作業は，一部自動化することも可能である。例えば，下記に説明されているように，PareViewステートファイルを準備しておき，スクリプトを使う方法などがある。

http://www.slideshare.net/MasashiImano/cae20140920-opencae-workshopatkansaiparaview

なお，ESI社が機能を追加して公開しているOpenFOAM+版では，計算時の処理の一環として，画像ファイルを作成することも可能である。

### ポスト処理出力の確認：streamLines ###

ディレクトリ postProcessing/ に streamLines ディレクトリが作成されていることを確認する。その下にある各時刻ディレクトリの中にあるファイルを開いて内部を確認する。

これらのファイルは，vtk形式のファイルである。このままでは，画像として表示されない。ParaViewから読み込むと，画像として表示できる。

[手順一覧に戻る](#tableOfContents)


<a name="section5"></a>
## 領域内の平均値，最小値，最大値を求める ##

volRegion で 平均値，最大値，最小値 をファイルに書き出す。書き出したファイルから，時刻履歴グラフを作成する。


### 平均値，最小値，最大値出力のためにsystem/volRegionファイルを作成 ###

例題のsystemディレクトリに，残差出力設定用ファイル volRegion を作成する。作成したvolRegionファイルに下記を書き込む。ここでは，volRegionAverageT，volRegionMin，volRegionMaxという3つの名前で，体積平均値，最小値，最大値を出力するように指定する。

```
volRegionAverageT
{
    type            volRegion;
    libs ("libfieldFunctionObjects.so");

    //enabeld true;
    writeControl timeStep;
    writeInterval 50;

    log             true;
    writeFields     no; //true;
    regionType      all; //cellZone;
    //name            c0;
    operation       volAverage;
    //weightField     alpha1;
    fields
    (
        T
        //U
    );
}

volRegionMin
{
    type            volRegion;
    libs ("libfieldFunctionObjects.so");

    //enabeld true;
    writeControl timeStep;
    writeInterval 50;

    log             true;
    writeFields     no; //true;
    regionType      all; //cellZone;
    //name            c0;
    operation       min;
    //weightField     alpha1;
    fields
    (
        T
        U
    );
}

volRegionMax
{
    type            volRegion;
    libs ("libfieldFunctionObjects.so");

    //enabeld true;
    writeControl timeStep;
    writeInterval 50;

    log             true;
    writeFields     no; //true;
    regionType      all; //cellZone;
    //name            c0;
    operation       max;
    //weightField     alpha1;
    fields
    (
        T
        U
    );
}
```

### 平均値，最小値，最大値出力のためにsystem/controlDictファイルを修正 ###

作成したcuttingPlaneファイルおよびstreamLinesファイルの内容を実行時に参照するように，system/controlDictファイルを修正する。controlDictファイルの下部に下記を追加する。

```
functions
{
    #include "residuals"
    #include "probes"
    #include "cuttingPlane"
    #include "streamLines"
    #include "volRegion"
}
```

### ポスト処理出力の確認：volRegion ###

ディレクトリ postProcessing/ に volRegionAverageT/，volRegionMin/，volRegionMax/ ディレクトリが作成されていることを確認する。その下にある0ディレクトリの中にあるファイルを開いて内部を確認する。

端末から下記コマンドを実行し，温度の時刻歴ファイルをグラフ表示する。

> foamMonitor postProcessing/volRegionAverageT/0/volRegion.dat

表示されるグラフを下図に示す。

| <img src=./img/averageTHistory.png alt="volRegionで抽出した体積平均温度履歴のグラフ" title="volRegionで抽出した体積平均温度履歴のグラフ" width=600px> |
|:-------:|
|  図 　volRegionによる体積平均温度履歴のグラフ  |


[手順一覧に戻る](#tableOfContents)


<a name="section6"></a>
## メモ

gnuplot

montage

foamLog ユーティリティ

実行結果を確認する。ParaViewで，中心断面を見る。流線を書く。これと同等のものを，自動化することを目指していく。

wallHeatFluxユーティリティを実行する。熱収支について考える。熱収支の時間変化，温度の時間変化を知りたい。自動化を目指す。（平均温度も見たい　of3はcellSource，of4ではvolRegionを使う　　）

https://github.com/OpenFOAM/OpenFOAM-4.x/blob/master/src/functionObjects/field/fieldValues/volRegion/volRegion.H

https://github.com/OpenFOAM/OpenFOAM-4.x/blob/75ea76187b82cffbdc84907e7e57e02997e5025b/applications/utilities/postProcessing/sampling/probeLocations/probesDict

ポスト処理として，熱収支を考える　→　log から　ファイル操作でgnuplot用ファイルに変換してグラフ化

楽になるために，functionObjectの改造を考える

https://github.com/snaka-dev/OF_logFileHandling/blob/master/logFileHandling.md

[手順一覧に戻る](#tableOfContents)




<a name="section7"></a>
## For OpenFOAM 3

### residualsファイル for OpenFOAM3 ###

```
residuals
{
    type            residuals;
    functionObjectLibs ("libutilityFunctionObjects.so");

    // OpenFOAM 3
    enabled         true;
    outputControl   timeStep;
    outputInterval  50; //1;

    // OpenFOAM 4
    //writeControl   timeStep;
    //writeInterval  50;

    fields ( h k omega p_rgh); // U

}
```

### probesファイル for OpenFOAM3 ###

```
probes
{
    type probes;
    functionObjectLibs ("libsampling.so");
    enabled         true;

    // OpenFOAM 3
    outputControl   timeStep;
    outputInterval  50; //1;

    // OpenFOAM 4
    //writeControl timeStep;
    //writeInterval 50;

    // Fields to be probed. runTime modifiable!
    fields
    (
        p_rgh T U k omega
    );

    // Locations to be probed. runTime modifiable!
    probeLocations
    (
        (0.038 1.0901 0.0) //center
        (0.038 0.654 0.0)  // y=0.3h

        (0.001 1.0901 0.0) // x=0.001, y=0.5h
        (0.010 1.0901 0.0) // x=0.010, y=0.5h
        (0.066 1.0901 0.0) // x=0.066, y=0.5h
        (0.075 1.0901 0.0) // x=0.075, y=0.5h

        (0.075 2.17 0.0) // x=0.075, y=2.18-0.01
        (0.001 0.01 0.0) // x=0.001, y=0.01

    );
}
```

### cuttingPlaneファイル for OpenFOAM3 ###

```
cuttingPlane
{
    type            surfaces;
    libs ("libsampling.so");

    // OpenFOAM 3
    enabled         true;
    outputControl   outputTime;

    // OpenFOAM 4
    //writeControl    writeTime;

    surfaceFormat   vtk;
    fields          ( p p_rgh U T omega );

    interpolationScheme cellPoint;

    surfaces
    (
        zNormal
        {
            type            cuttingPlane;
            planeType       pointAndNormal;
            pointAndNormalDict
            {
                basePoint       (0 0 0);
                normalVector    (0 0 1);
            }
            interpolate     true;
        }

        yNormal
        {
            type            cuttingPlane;
            planeType       pointAndNormal;
            pointAndNormalDict
            {
                basePoint       (0 0 0);
                normalVector    (0 1 0);
            }
            interpolate     true;
        }

    );
}
```

### streamLinesファイル for OpenFOAM3 ###

```
streamLines
{
    type            streamLine;

    // OpenFOAM 3
    functionObjectLibs ("libfieldFunctionObjects.so");

    // OpenFOAM 4
    //libs ("libfieldFunctionObjects.so");

    //OpenFOAM 3
    UName           U;
    outputControl   outputTime;
    // outputInterval 10;

    // OpenFOAM 4
    //writeControl    writeTime;
    // writeInterval 10;

    setFormat       vtk; //gnuplot; //xmgr; //raw; //jplot; //csv; //ensight;

    // Tracked forwards (+U) or backwards (-U)
    trackForward    true;

    // Names of fields to sample. Should contain above velocity field!
    fields (p k T U);

    // Steps particles can travel before being removed
    lifeTime        10000;

    //- Specify either absolute length of steps (trackLength) or a number
    //  of subcycling steps per cell (nSubCycle)

        // Size of single track segment [m]
        //trackLength 1e-3;

        // Number of steps per cell (estimate). Set to 1 to disable subcycling.
        nSubCycle 5;


    // Cloud name to use
    cloudName       particleTracks;

    // Seeding method.
    seedSampleSet   uniform;  //cloud; //triSurfaceMeshPointSet;

    uniformCoeffs
    {
        type        uniform;
        axis        x;  //distance;

        // Note: tracks slightly offset so as not to be on a face
        start       (0.0001 1.0901 0.0001);
        end         (0.0759 1.0901 0.0001);
        nPoints     20;
    }
}
```

### volRegionファイル for OpenFOAM3 ###

OpenFOAM3系では cellSource タイプを使用する。OpenFOAM 4系に合わせるため，ファイル名はvolRegionのままとするが，ファイル内のtype は cellSource としていることに注意。

```
volRegionAverageT
{
    type            cellSource;
    functionObjectLibs ("libfieldFunctionObjects.so");
    enabled         true;
    outputControl   outputTime;
    log             true;
    valueOutput     no; //true;

    source          all; //cellZone;
    //sourceName      c0;
    operation       volAverage;

    fields
    (
        T
    );
}

volRegionMin
{
    type            cellSource;
    functionObjectLibs ("libfieldFunctionObjects.so");
    enabeld         true;
    outputControl   outputTime;
    log             true;
    valueOutput     no; //true;
    //writeVolume     no; //true;
    source          all; //cellZone;
    //sourceName            c0;
    operation       min;
    //weightField     alpha1;
    fields
    (
        T
        //U
    );
}

volRegionMax
{
    type            cellSource;
    functionObjectLibs ("libfieldFunctionObjects.so");
    enabeld         true;
    outputControl   outputTime;
    log             true;
    valueOutput     no; //true;
    //writeVolume     no; //true;
    source          all; //cellZone;
    //sourceName            c0;
    operation       max;
    //weightField     alpha1;
    fields
    (
        T
        //U
    );
}
```

### createGraphsスクリプト for OpenFOAM3 ###


98行目の最終結果時刻取得方法を変更する。
OpenFOAM 4では，sampleによる結果がpostProcessing/sample内に作成される。
OpenFOAM 3 では，sampleの結果がpostProcessing/sets/に書かれる。このsets/ディレクトリには，streamLinesディレクトリも作成される。オリジナルのcreateGraphsスクリプトでは，最終結果の時刻をsetsディレクトリ内の名前から取り出すことになっており，streamLinesディレクトリが存在すると時刻が正確に取り出せない。そのため，最終時刻の取り出しに foamListTimes ユーティリティを利用するように修正した。

```
#!/bin/sh
#------------------------------------------------------------------------------
# =========                 |
# \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
#  \\    /   O peration     |
#   \\  /    A nd           | Copyright (C) 2011-2015 OpenFOAM Foundation
#    \\/     M anipulation  |
#-------------------------------------------------------------------------------
# License
#     This file is part of OpenFOAM.
#
#     OpenFOAM is free software: you can redistribute it and/or modify it
#     under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
#     ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#     FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
#     for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.
#
# Script
#     createGraphs
#
# Description
#     Creates .eps graphs of OpenFOAM results vs experiment for the buoyant
#     cavity case
#
#------------------------------------------------------------------------------

createEpsT()
{
    index=$1
    OF=$2
    EXPT=$3

    gnuplot<<EOF
    set terminal postscript eps color enhanced
    set output "OF_vs_EXPT_T$i.eps"
    set xlabel "Channel width, x / [m]"
    set ylabel "Temperature / [K]"
    set grid
    set key left top
    set size 0.6, 0.6
    set xrange [0:0.08]
    set yrange [285:310]
    plot \
        "$EXPT" u (\$1/1000):(\$2+273.15) title "Expt 0.$index" \
        with points lt 1 pt 6, \
        "$OF" title "OpenFOAM 0.$index" with lines linetype -1
EOF
}


createEpsU()
{
    index=$1
    OF=$2
    EXPT=$3

    gnuplot<<EOF
    set terminal postscript eps color enhanced
    set output "OF_vs_EXPT_U$i.eps"
    set xlabel "Channel width, x / [m]"
    set ylabel "Vertical velocity component, Uy / [m/s]"
    set grid
    set key left top
    set size 0.6, 0.6
    set xrange [0:0.08]
    set yrange [-0.2:0.2]
    plot \
        "$EXPT" u (\$1/1000):(\$2) title "Expt 0.$index" \
        with points lt 1 pt 6, \
        "$OF" u 1:3 title "OpenFOAM 0.$index" with lines linetype -1
EOF
}


# test if gnuplot exists on the system
if ! which gnuplot > /dev/null 2>&1
then
    echo "gnuplot not found - skipping graph creation" >&2
    exit 1
fi

SETSDIR="../postProcessing/sets"

if [ ! -d $SETSDIR ]
then
    echo "createGraphs: results sets not available in directory $SETSDIR"
    exit 0
fi

# paths to data
#LATESTTIME=`ls $SETSDIR`
LATESTTIME=`foamListTimes -case .. -latestTime`
OFDATAROOT=$SETSDIR/$LATESTTIME

EXPTDATAROOT=./exptData

# generate temperature profiles
TSets="1 3 4 5 6 7 9"
for i in $TSets
do
    echo "    processing temperature profile at y/yMax of 0.$i" \
        > log.createGraphs 2>&1

    OF="$OFDATAROOT/y0.${i}_T.xy"
    EXPT="$EXPTDATAROOT/mt_z0_${i}0_lo.dat"

    createEpsT $i $OF $EXPT
done


# generate velocity profiles
USets="1 3 4 5 6 7 9"
for i in $USets
do
    echo "    processing velocity profile at y/yMax of 0.$i" \
        > log.createGraphs 2>&1

    OF="$OFDATAROOT/y0.${i}_U.xy"
    EXPT="$EXPTDATAROOT/mv_z0_${i}0_lo.dat"

    createEpsU $i $OF $EXPT
done

echo Done

#------------------------------------------------------------------------------
```

[手順一覧に戻る](#tableOfContents)
