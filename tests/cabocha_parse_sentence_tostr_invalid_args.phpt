--TEST--
cabocha_parse_sentence with invalid arguments
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = '吾輩はここで始めて人間というものを見た。';
$str = cabocha_parse_sentence_tostr($input, '-n');

echo $str === false, PHP_EOL;
?>
--EXPECTF--
Warning: cabocha_parse_sentence_tostr(): `-n` requires an argument in %s
1
