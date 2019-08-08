# simpleMesh-with-energy
「ns3によるネットワークシミュレーション」の第8章開発事例1をns-3.29に対応させたコード

## Environment
- Ubuntu 16.04
- ns3.29

## Corrections
- mesh-radio-energy-model-helper.ccにあったSetEnergyDepletionCallback()とSetEnergyRechargedCallback()はメッシュモデルに適応できなさそうだったので削除した。
- シナリオスクリプトはすべてscratch/simpleMesh-with-energyというフォルダにまとめてしまった
- simpleMeshTest.ccにあるReportRtables()は現在のns-3.29にはないため削除した。代わりにテーブル情報のトレースソースがあるので、必要な人はこちらを利用するといいかもしれない。
- simpleMeshTest.ccのSetUpTcpApplication()で、BGRノードがAnyAddressからパケット受け付けるソケットを何度も開いていてエラーが出るので修正。
- 初期バッテリー量initialEnergyが0.1(J)となっていたが、これでは数秒でバッテリー切れを起こしてしまうため50(J)とした
- 変更点の詳細はコミットログを参照のこと

## Usage
githubアカウントがあれば、以下のコマンドでコードをローカルにダウンロードできる。

```sh
$ git clone git@github.com:dorapon2000/ns3-simpleMesh-with-energy.git
$ cd ns3-simpleMesh-with-energy
```

ns3のフォルダがホームディレクトリ直下にあれば、以下のコマンドでコードを反映できる(上書きなので注意)。

```sh
$ chmode +x update.sh
$ ./update.sh
```

ns3フォルダ内に移動してビルドが成功することを祈る。

```sh
$ ./waf
```

ns3ルートフォルダ内に「datatata」というフォルダを作成する。そこに実行結果ファイルが出力される。そして、実際に動かす。

```sh
$ mkdir datatata
$ ./waf --run "simpleMesh-with-energy"
```
