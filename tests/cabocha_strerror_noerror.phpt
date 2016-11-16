--TEST--
cabocha_strerror (has global error)
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$cabocha = cabocha_new();
$error = cabocha_strerror();
echo $error;
$error = cabocha_strerror($cabocha);
echo $error;
?>
--EXPECT--
