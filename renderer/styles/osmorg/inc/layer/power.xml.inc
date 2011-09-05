<Layer name="power_towers" status="on" srs="&osm2pgsql_projection;">
	<StyleName>power_towers</StyleName>
	<Datasource>
		<Parameter name="table">
		(
		
			SELECT way
			  FROM &prefix;_point
			 WHERE tags->'power' = 'tower'
		
		) as subq
		</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>
<Style name="power_towers">
	<Rule>
		&maxscale_zoom14;
		&minscale_zoom14;
		
		<PointSymbolizer file="&symbols;/misc/power/tower_3x3.png" />
	</Rule>
		<Rule>
		&maxscale_zoom15;
		&minscale_zoom16;
		
		<PointSymbolizer file="&symbols;/misc/power/tower_5x5.png" />
	</Rule>
		<Rule>
		&maxscale_zoom17;
		
		<PointSymbolizer file="&symbols;/misc/power/tower_7x7.png" />
	</Rule>
</Style>



<Layer name="power_poles" status="on" srs="&osm2pgsql_projection;">
	<StyleName>power_poles</StyleName>
	
	<Datasource>
		<Parameter name="table">
		(
		
			SELECT way
			  FROM &prefix;_point
			 WHERE tags->'power' = 'pole'
		
		) as subq
		</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>
<Style name="power_poles">
	<Rule>
		&maxscale_zoom16;
		
		<PointSymbolizer file="&symbols;/misc/power/pole.png" />
	</Rule>
</Style>
