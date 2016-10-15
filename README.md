# PHP extension to CaboCha

日本語係り受け解析器 [CaboCha](https://taku910.github.io/cabocha/) を PHP から利用するための拡張モジュールです。現時点では文字列から係り受け木を得る機能のみ実装されています。

## 動作環境

この拡張モジュールの動作には以下が必要です。
- PHP 7.0
- CaboCha
    - CaboCha 0.69 で動作を確認しています

## インストール

通常の PHP 拡張モジュールと同様にインストールできます。
```
$ git clone https://github.com/y-uti/php-cabocha.git
$ cd php-cabocha
$ phpize
$ ./configure
$ make
$ sudo make install
```

## 使い方

`cabocha_parse_sentence` 関数で文字列を解析できます。
```
<?php
$tree = cabocha_parse_sentence('吾輩はここで始めて人間というものを見た。');
```

第二引数に `cabocha` コマンドに渡すオプションを指定できます。
```
<?php
$tree = cabocha_parse_sentence('寒月と、根津、上野、池の端、神田辺を散歩。', '-n2');
```

複数の文字列を連続して解析するには `cabocha_parse` 関数を利用できます。
```
<?php
$cabocha = cabocha_new('-n2');
$tree = cabocha_parse($cabocha, '吾輩はここで始めて人間というものを見た。');
$tree = cabocha_parse($cabocha, '寒月と、根津、上野、池の端、神田辺を散歩。');
```

### 解析結果

解析結果は PHP の配列形式のデータとして格納されます。これは CaboCha の `cabocha_sparse_totree` 関数から得られた `cabocha_tree_t` 構造体を PHP の配列に詰め替えたものです。

`tests/cabocha_parse.phpt` に簡単な例があります。
