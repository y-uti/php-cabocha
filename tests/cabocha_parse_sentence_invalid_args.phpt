--TEST--
cabocha_parse_sentence with invalid arguments
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = '吾輩はここで始めて人間というものを見た。';
$tree = cabocha_parse_sentence($input, '-n');

echo $tree === false, PHP_EOL;
?>
--EXPECTF--
Warning: cabocha_parse_sentence(): `-n` requires an argument in %s
1
