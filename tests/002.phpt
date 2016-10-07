--TEST--
Check for cabocha_parse function with options
--SKIPIF--
<?php if (!extension_loaded("cabocha")) print "skip"; ?>
--FILE--
<?php
$input = '寒月と、根津、上野、池の端、神田辺を散歩。';
$tree = cabocha_parse($input, "-n2");

echo $tree['sentence'], "\n";
foreach ($tree['chunk'] as $i => $chunk) {
    $tokens = array_slice($tree['token'], $chunk['token_pos'], $chunk['token_size']);
    $text = '';
    foreach ($tokens as $token) {
        $text .= $token['surface'];
        if (($ne = $token['ne']) !== "O") {
            $text .= "<$ne>";
        }

    }
    $link = $chunk['link'];
    echo "$i: $text ($link)\n";
}
?>
--EXPECT--
寒月と、根津、上野、池の端、神田辺を散歩。
0: 寒月と、 (5)
1: 根津<B-PERSON>、 (2)
2: 上野<B-PERSON>、 (3)
3: 池の (4)
4: 端、 (5)
5: 神田<B-PERSON>辺<I-PERSON>を (6)
6: 散歩。 (-1)
