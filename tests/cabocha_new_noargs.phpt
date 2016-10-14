--TEST--
cabocha_new with no arguments
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$cabocha = cabocha_new();
echo get_resource_type($cabocha), PHP_EOL;
?>
--EXPECT--
cabocha
