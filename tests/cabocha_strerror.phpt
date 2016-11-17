--TEST--
cabocha_strerror
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = <<<END_OF_INPUT
* 0
END_OF_INPUT;
$input .= PHP_EOL;

$cabocha = cabocha_new('-I2');
$tree = @cabocha_parse($cabocha, $input);
$error = cabocha_strerror($cabocha);
echo $error;
?>
--EXPECTF--
format error: [* 0
]
