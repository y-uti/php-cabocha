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

$tree = cabocha_parse_sentence($input, '-I2');
echo $tree === false;
?>
--EXPECTF--
Warning: cabocha_parse_sentence(): format error: [* 0
]  in %s
1
