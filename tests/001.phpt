--TEST--
Check for cabocha_parse function
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = '吾輩はここで始めて人間というものを見た。';
$tree = cabocha_parse($input);

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
