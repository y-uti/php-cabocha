--TEST--
cabocha_new with arguments
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$cabocha = cabocha_new("-n2");
echo get_resource_type($cabocha), PHP_EOL;
?>
--EXPECT--
cabocha
