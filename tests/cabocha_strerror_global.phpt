--TEST--
cabocha_strerror (has global error)
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$cabocha = @cabocha_new('-n');
$error = cabocha_strerror();
echo $error;
?>
--EXPECT--
`-n` requires an argument
