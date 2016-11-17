--TEST--
cabocha_parse (invalid input)
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = <<<END_OF_INPUT
* 0
END_OF_INPUT;
$input .= PHP_EOL;

$str = cabocha_parse_sentence_tostr($input, '-I2');
echo $str === false;
?>
--EXPECTF--
Warning: cabocha_parse_sentence_tostr(): format error: [* 0
]  in %s
1
