<?php

if(!isset($argv[1]))
	die("supply way-id as argument\n");

$id = (int)$argv[1];

$url = "http://www.openstreetmap.org/api/0.6/way/$id/history";
fwrite(STDERR, "loading $url\n");
$wayhist = new DOMDocument();
$wayhist->load($url);

$nodeshist = array();
foreach($wayhist->getElementsByTagName('way') as $way)
{
	foreach($way->getElementsByTagName('nd') as $nd)
	{
		$ref = (int)$nd->getAttribute('ref');
		if(isset($nodeshist[$ref]))
			continue;

		$url = "http://www.openstreetmap.org/api/0.6/node/$ref/history";
		fwrite(STDERR, "loading $url\n");
		$nodeshist[$ref] = new DOMDocument();
		$nodeshist[$ref]->load($url);
	}
}

fwrite(STDERR, "sorting nodes\n");
ksort($nodeshist);

fwrite(STDERR, "assembling document\n");
$mergedoc = new DOMDocument();
$mergedoc->loadXML('<osm version="0.6" generator="Way-with-fistory-fetcher" copyright="OpenStreetMap and contributors" attribution="http://www.openstreetmap.org/copyright" license="http://opendatacommons.org/licenses/odbl/1-0/" />');
$rootnode = $mergedoc->documentElement;

foreach($nodeshist as $nodehist)
{
	foreach($nodehist->getElementsByTagName('node') as $node)
	{
		$node = $mergedoc->importNode($node, true);
		$rootnode->appendChild($node);
	}
}

foreach($wayhist->getElementsByTagName('way') as $way)
{
	$way = $mergedoc->importNode($way, true);
	$rootnode->appendChild($way);
}

$mergedoc->formatOutput = true;
echo $mergedoc->saveXML();
