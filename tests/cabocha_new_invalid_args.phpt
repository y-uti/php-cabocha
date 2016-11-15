--TEST--
cabocha_new with invalid arguments
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$cabocha = cabocha_new('-n');
echo $cabocha === false, PHP_EOL;
?>
--EXPECTF--
Warning: cabocha_new(): `-n` requires an argument in %s
1
