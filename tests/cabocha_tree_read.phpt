--TEST--
cabocha_tree_read
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = '吾輩はここで始めて人間というものを見た。';
$input_layer = CABOCHA_INPUT_RAW_SENTENCE;
$tree = cabocha_tree_read($input, $input_layer);

echo $tree['sentence'], PHP_EOL;
?>
--EXPECT--
吾輩はここで始めて人間というものを見た。
