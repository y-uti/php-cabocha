--TEST--
cabocha_destroy
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$cabocha = cabocha_new();
cabocha_destroy($cabocha);
echo get_resource_type($cabocha), PHP_EOL;
?>
--EXPECT--
Unknown
