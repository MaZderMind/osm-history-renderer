include("lib/base.js");

importer.node().on(function(node) {
    print("node: "+node.id+"v"+node.version);
});

importer.node().tag('amenity').on(function(node) {
    print("amenity: "+node.id+"v"+node.version);
});

importer.node().tag({'amenity': 'bar'}).on(function(node) {
    print("bar: "+node.id+"v"+node.version);
});

importer.node().tag({'amenity': ['bar', 'restaurant']}).on(function(node) {
    print("cool place: "+node.id+"v"+node.version);
});

/*
importer.node().tag('amenity').
    when().
        inside(...).on(..)
    otherwise().
        inside(...).on(..);
*/

