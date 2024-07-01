#!/usr/bin/php
<?php

require __DIR__ . '/../vendor/autoload.php';

use Statusengine\Client;

$Client = new Client();

$query = 1;
while(1){
    $result = $Client->getServicestatus();

    if(!is_array($result)){
        echo "Error: Broken result from Naemon\n";
        print_r($result);
        die();
    }

    echo "Query: #$query. Result size: ".sizeof($result)."\n";
    $query++;

    $result = $Client->getServicestatus('hplj2605dn', 'PING');
    if(!is_array($result) || sizeof($result) != 6){
        echo "Error: Broken result from Naemon or too many records\n";
        print_r($result);
        die();
    }
    $query++;

    usleep(250*1000);
}
