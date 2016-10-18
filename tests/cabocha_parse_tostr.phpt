--TEST--
cabocha_parse
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$cabocha = cabocha_new();
$input = '吾輩はここで始めて人間というものを見た。';
$str = cabocha_parse_tostr($cabocha, $input);

echo $str;
?>
--EXPECT--
吾輩は---------D
      ここで-D     |
        始めて-D   |
      人間という-D |
            ものを-D
              見た。
EOS
