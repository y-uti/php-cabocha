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

$cabocha = cabocha_new('-I2');
$tree = cabocha_parse($cabocha, $input);
echo $tree === false;
?>
--EXPECTF--
Warning: cabocha_parse(): format error: [* 0
]  in %s
1
