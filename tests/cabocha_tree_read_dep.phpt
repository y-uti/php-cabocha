--TEST--
cabocha_tree_read_dep
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = <<<END_OF_INPUT
* 0 5D 0/1 -1.514009
吾輩	名詞,代名詞,一般,*,*,*,吾輩,ワガハイ,ワガハイ
は	助詞,係助詞,*,*,*,*,は,ハ,ワ
* 1 2D 0/1 1.311423
ここ	名詞,代名詞,一般,*,*,*,ここ,ココ,ココ
で	助詞,格助詞,一般,*,*,*,で,デ,デ
* 2 3D 0/1 0.123057
始め	動詞,自立,*,*,一段,連用形,始める,ハジメ,ハジメ
て	助詞,接続助詞,*,*,*,*,て,テ,テ
* 3 4D 0/1 1.440044
人間	名詞,一般,*,*,*,*,人間,ニンゲン,ニンゲン
という	助詞,格助詞,連語,*,*,*,という,トイウ,トユウ
* 4 5D 0/1 -1.514009
もの	名詞,非自立,一般,*,*,*,もの,モノ,モノ
を	助詞,格助詞,一般,*,*,*,を,ヲ,ヲ
* 5 -1D 0/1 0.000000
見	動詞,自立,*,*,一段,連用形,見る,ミ,ミ
た	助動詞,*,*,*,特殊・タ,基本形,た,タ,タ
。	記号,句点,*,*,*,*,。,。,。
EOS
END_OF_INPUT;
$input .= PHP_EOL;
$input_layer = CABOCHA_INPUT_DEP;
$tree = cabocha_tree_read($input, $input_layer);

echo $tree['sentence'], "\n";
foreach ($tree['chunk'] as $i => $chunk) {
    $tokens = array_slice($tree['token'], $chunk['token_pos'], $chunk['token_size']);
    $text = '';
    foreach ($tokens as $token) {
        $text .= $token['surface'];
    }
    $link = $chunk['link'];
    echo "$i: $text ($link)\n";
}
?>
--EXPECT--
吾輩はここで始めて人間というものを見た。
0: 吾輩は (5)
1: ここで (2)
2: 始めて (3)
3: 人間という (4)
4: ものを (5)
5: 見た。 (-1)
