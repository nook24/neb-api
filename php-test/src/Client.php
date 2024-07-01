<?php

namespace Statusengine;

class Client{

    private $client;

    public function __construct($base_url = 'http://localhost:8000'){
        $this->client = new \GuzzleHttp\Client([
            'base_uri' => $base_url,
            'timeout'  => 2.0,
        ]);
    }

    public function getServicestatus(?string $hostname = null, ?string $service_description = null): array {
        $query = [];

        if($hostname && $service_description){
            $query =  [
                'hostname' => $hostname,
                'service_description' => $service_description
            ];
        }

        $response = $this->client->request('GET', '/servicestatus', [
            'query' => $query
        ]);

        return json_decode($response->getBody()->getContents(), true);
    }

}

