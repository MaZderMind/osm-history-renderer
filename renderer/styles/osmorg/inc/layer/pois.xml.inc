&layer-pois-accommodation;
&layer-pois-amenity;
&layer-pois-barrier;
&layer-pois-education;
&layer-pois-food;
&layer-pois-health;
&layer-pois-landuse;
&layer-pois-money;
&layer-pois-place_of_worship;
&layer-pois-poi;
&layer-pois-power;
&layer-pois-shopping;
&layer-pois-sport;
&layer-pois-tourist;
&layer-pois-transport;
&layer-pois-water;

<Layer name="pois-points" status="on" srs="&dbProjection;">
	<StyleName>pois-accommodation</StyleName>
	<StyleName>pois-amenity</StyleName>
	<!--StyleName>barrier</StyleName>
	<StyleName>education</StyleName>
	<StyleName>food</StyleName>
	<StyleName>health</StyleName>
	<StyleName>landuse</StyleName>
	<StyleName>money</StyleName>
	<StyleName>place_of_worship</StyleName>
	<StyleName>poi</StyleName>
	<StyleName>power</StyleName>
	<StyleName>shopping</StyleName>
	<StyleName>sport</StyleName>
	<StyleName>tourist</StyleName>
	<StyleName>transport</StyleName>
	<StyleName>water</StyleName-->

	<Datasource>
		<Parameter name="table">
		(
		
			SELECT way, tags->'amenity' AS amenity, tags->'shop' AS shop, tags->'tourism' AS tourism,
				tags->'highway' AS highway, tags->'man_made' AS man_made, tags->'access' AS access, 
				tags->'religion' AS religion, tags->'waterway' AS waterway, tags->'lock' AS lock, 
				tags->'historic' AS historic, tags->'leisure' AS leisure
			  FROM &prefix;_point
			 WHERE tags ?| ARRAY['amenity', 'shop', 'tourism', 'highway', 'man_made', 'access', 'religion', 'waterway', 'lock', 'historic', 'leisure']
			   AND &date;
			
		) as subq
		</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>

<Layer name="pois-polygons" status="on" srs="&dbProjection;">
	<StyleName>pois-accommodation</StyleName>
	<!--StyleName>amenity</StyleName>
	<StyleName>barrier</StyleName>
	<StyleName>education</StyleName>
	<StyleName>food</StyleName>
	<StyleName>health</StyleName>
	<StyleName>landuse</StyleName>
	<StyleName>money</StyleName>
	<StyleName>place_of_worship</StyleName>
	<StyleName>poi</StyleName>
	<StyleName>power</StyleName>
	<StyleName>shopping</StyleName>
	<StyleName>sport</StyleName>
	<StyleName>tourist</StyleName>
	<StyleName>transport</StyleName>
	<StyleName>water</StyleName-->

	<Datasource>
		<Parameter name="table">
		(
		
			SELECT center AS way, tags->'amenity' AS amenity, tags->'shop' AS shop, tags->'tourism' AS tourism,
				tags->'highway' AS highway, tags->'man_made' AS man_made, tags->'access' AS access, 
				tags->'religion' AS religion, tags->'waterway' AS waterway, tags->'lock' AS lock, 
				tags->'historic' AS historic, tags->'leisure' AS leisure
			  FROM &prefix;_polygon
			 WHERE tags ?| ARRAY['amenity', 'shop', 'tourism', 'highway', 'man_made', 'access', 'religion', 'waterway', 'lock', 'historic', 'leisure']
			   AND &date;
			
		) as subq
		</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>
