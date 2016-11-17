--TEST--
cabocha_parse_tostr (invalid input)
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = <<<END_OF_INPUT
* 0
END_OF_INPUT;
$input .= PHP_EOL;

$cabocha = cabocha_new('-I2');
$str = cabocha_parse_tostr($cabocha, $input);
echo $str === false;
?>
--EXPECTF--
Warning: cabocha_parse_tostr(): format error: [* 0
]  in %s
1
