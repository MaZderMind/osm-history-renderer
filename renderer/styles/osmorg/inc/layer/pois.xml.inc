<Layer name="pois-points" status="on" srs="&dbProjection;">
	<StyleName>pois-accommodation</StyleName>
	<StyleName>pois-amenity</StyleName>
	<StyleName>pois-barrier</StyleName>
	<StyleName>pois-education</StyleName>
	<StyleName>pois-food</StyleName>
	<StyleName>pois-health</StyleName>
	<StyleName>pois-money</StyleName>
	<StyleName>pois-place_of_worship</StyleName>
	<StyleName>pois-manmade</StyleName>
	<!--StyleName>pois-power</StyleName-->
	<StyleName>pois-shopping</StyleName>
	<StyleName>pois-sport</StyleName>
	<StyleName>pois-tourist</StyleName>
	<!--StyleName>pois-transport</StyleName-->
	<!--StyleName>pois-water</StyleName-->

	<Datasource>
		<Parameter name="table">
		(
		
			SELECT way, tags->'amenity' AS amenity, tags->'shop' AS shop, tags->'tourism' AS tourism,
				tags->'highway' AS highway, tags->'man_made' AS man_made, tags->'access' AS access, 
				tags->'religion' AS religion, tags->'waterway' AS waterway, tags->'lock' AS lock, 
				tags->'historic' AS historic, tags->'leisure' AS leisure, tags->'barrier' AS barrier, 
				tags->'cuisine' AS cuisine, tags->'sport' AS sport, tags->'building' AS building, 
				tags->'name' AS name, tags->'natural' AS natural
			  FROM &prefix;_point
			 WHERE tags ?| ARRAY['amenity', 'shop', 'tourism', 'highway', 'man_made', 'access', 'waterway', 'lock', 'historic', 'leisure', 'barrier', 'sport', 'natural']
			   AND &date;
			
		) as subq
		</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>

<Layer name="pois-polygons" status="on" srs="&dbProjection;">
	<StyleName>pois-accommodation</StyleName>
	<StyleName>pois-amenity</StyleName>
	<StyleName>pois-barrier</StyleName>
	<StyleName>pois-education</StyleName>
	<StyleName>pois-food</StyleName>
	<StyleName>pois-health</StyleName>
	<StyleName>pois-money</StyleName>
	<StyleName>pois-place_of_worship</StyleName>
	<StyleName>pois-manmade</StyleName>
	<!--StyleName>pois-power</StyleName-->
	<StyleName>pois-shopping</StyleName>
	<StyleName>pois-sport</StyleName>
	<StyleName>pois-tourist</StyleName>
	<!--StyleName>pois-transport</StyleName-->
	<!--StyleName>pois-water</StyleName-->

	<Datasource>
		<Parameter name="table">
		(
		
			SELECT center AS way, tags->'amenity' AS amenity, tags->'shop' AS shop, tags->'tourism' AS tourism,
				tags->'highway' AS highway, tags->'man_made' AS man_made, tags->'access' AS access, 
				tags->'religion' AS religion, tags->'waterway' AS waterway, tags->'lock' AS lock, 
				tags->'historic' AS historic, tags->'leisure' AS leisure, tags->'barrier' AS barrier, 
				tags->'cuisine' AS cuisine, tags->'sport' AS sport, tags->'building' AS building, 
				tags->'name' AS name, tags->'natural' AS natural
			  FROM &prefix;_polygon
			 WHERE tags ?| ARRAY['amenity', 'shop', 'tourism', 'highway', 'man_made', 'access', 'waterway', 'lock', 'historic', 'leisure', 'barrier', 'sport', 'natural']
			   AND &date;
			
		) as subq
		</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>

&layer-pois-accommodation;
&layer-pois-amenity;
&layer-pois-barrier;
&layer-pois-education;
&layer-pois-food;
&layer-pois-health;
&layer-pois-money;
&layer-pois-place_of_worship;
&layer-pois-manmade;
&layer-pois-power;
&layer-pois-shopping;
&layer-pois-sport;
&layer-pois-tourist;
&layer-pois-transport;
&layer-pois-water;
