--TEST--
cabocha_parse_sentence with no options
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = '吾輩はここで始めて人間というものを見た。';
$str = cabocha_parse_sentence_tostr($input);

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
