#!/usr/bin/php


<?php 

$value = getenv('QUERY_STRING');

echo "<!DOCTYPE html>\r\n"
    . "<html>\r\n"
    . "  <head><title>My Page </title></head>\r\n"
    . "<body>\r\n"
    . "  <h1>Hello, my name is $value !</h1>\r\n"
    . "<p>I'm a web developer with a passion for learning new things.</p>\r\n"
    . "<h2>Some of my hobbies:</h2>\r\n"
    . "<ul>\r\n"
    . "      <li>Coding</li>\r\n"
    . "    <li>Photography</li>\r\n"
    . "    <li>Traveling</li>\r\n"
    . "</ul>\r\n"
    . "</body>\r\n"
    . "</html>\r\n";
?>
