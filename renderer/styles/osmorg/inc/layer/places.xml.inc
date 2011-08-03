<Layer name="placenames-large" status="on" srs="&dbProjection;">
	<StyleName>country</StyleName>
	<StyleName>state</StyleName>
	
	<Datasource>
		<Parameter name="table">
		(
		
			SELECT way, tags->'place' AS place, tags->'name' AS name, tags->'ref' AS ref
			  FROM &prefix;_point
			 WHERE tags->'place' IN ('country', 'state')
			   AND &date;
		
		) as subq
		</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>
<Style name="continent">
	<Rule>
		<Filter>[place] = 'continent'</Filter>
		
		&maxscale_zoom1;
		&minscale_zoom2;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="10" fill="&cPlaceName;" dy="0" halo-radius="1" wrap-width="20"/>
	</Rule>
</Style>
<Style name="country">
	<Rule>
		<Filter>[place] = 'country'</Filter>
		
		&maxscale_zoom2;
		&minscale_zoom3;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="8" fill="&cPlaceName;" dy="0" halo-radius="1" wrap-width="20"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'country'</Filter>
		
		&maxscale_zoom4;
		&minscale_zoom6;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="10" fill="&cPlaceName;" dy="0" halo-radius="1" wrap-width="20"/>
	</Rule>
</Style>
<Style name="state">
	<Rule>
		<Filter>[place] = 'state'</Filter>
		
		&maxscale_zoom4;
		&minscale_zoom4;
		
		<TextSymbolizer name="[ref]" fontset-name="oblique-fonts" size="9" fill="&cPlaceName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'state'</Filter>
		
		&maxscale_zoom5;
		&minscale_zoom6;
		
		<TextSymbolizer name="[name]" fontset-name="oblique-fonts" size="9" fill="&cPlaceName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'state'</Filter>
		
		&maxscale_zoom7;
		&minscale_zoom8;
		
		<TextSymbolizer name="[name]" fontset-name="oblique-fonts" size="11" fill="&cPlaceName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
</Style>





<Layer name="placenames-capital" status="on" srs="&dbProjection;">
	<StyleName>capital</StyleName>
	
	<Datasource>
		<Parameter name="table">
		(
		
			SELECT way, tags->'place' AS place, tags->'name' AS name
			  FROM &prefix;_point
			 WHERE tags->'place' IN ('city', 'metropolis', 'town') AND tags->'capital' = 'yes'
			   AND &date;
		
		) AS subq</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>
<Style name="capital">
	<Rule>
		&maxscale_zoom5;
		&minscale_zoom5;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="9" fill="&cCityName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		&maxscale_zoom6;
		&minscale_zoom8;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="11" fill="&cCityName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		&maxscale_zoom9;
		&minscale_zoom10;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="11" fill="&cCityName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		&maxscale_zoom11;
		&minscale_zoom14;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="14" fill="&cCityName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
</Style>





<Layer name="placenames-medium" status="on" srs="&dbProjection;">
	<StyleName>city</StyleName>
	<StyleName>town</StyleName>
	
	<Datasource>
		<Parameter name="table">
		(
		
			SELECT way, tags->'place' AS place, tags->'name' AS name
			  FROM &prefix;_point
			 WHERE tags->'place' IN ('city', 'metropolis', 'town', 'large_town', 'small_town')
			   AND (NOT tags ? 'capital' OR tags->'capital' != 'yes')
			   AND &date;
		
		) as subq
		</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>
<Style name="city">
	<Rule>
		<Filter>[place] = 'city' or [place]='metropolis'</Filter>
		
		&maxscale_zoom6;
		&minscale_zoom8;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="8" fill="&cCityName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'city' or [place]='metropolis'</Filter>
		
		&maxscale_zoom9;
		&minscale_zoom10;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="11" fill="&cCityName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'city' or [place]='metropolis'</Filter>
		
		&maxscale_zoom11;
		&minscale_zoom14;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="14" fill="&cCityName;" dy="0" halo-radius="1" wrap-width="0"/>
	</Rule>
</Style>
<Style name="town">
		<Rule>
		<Filter>[place] = 'town' or [place]='large_town' or [place]='small_town'</Filter>
		
		&maxscale_zoom9;
		&minscale_zoom10;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="8" fill="&cCityName;" halo-radius="1" wrap-width="20"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'town' or [place]='large_town' or [place]='small_town'</Filter>
		
		&maxscale_zoom11;
		&minscale_zoom13;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="10" fill="&cCityName;" halo-radius="1" wrap-width="20"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'town' or [place]='large_town' or [place]='small_town'</Filter>
		
		&maxscale_zoom14;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="14" fill="&cCityNameHighZoom;" halo-radius="1" wrap-width="20"/>
	</Rule>
</Style>





<Layer name="placenames-small" status="on" srs="&dbProjection;">
	<StyleName>suburb</StyleName>
	<StyleName>village</StyleName>
	<StyleName>hamlet</StyleName>
	
	<Datasource>
		<Parameter name="table">
		(
		
			SELECT way, tags->'place' AS place, tags->'name' AS name
			  FROM &prefix;_point
			 WHERE tags->'place' IN ('suburb', 'village', 'large_village', 'hamlet', 'locality', 'isolated_dwelling', 'farm')
			   AND &date;
		
		) as subq
		</Parameter>
		
		&datasource-settings;
	</Datasource>
</Layer>
<Style name="suburb">
	<Rule>
		<Filter>[place] = 'suburb'</Filter>
		
		&maxscale_zoom12;
		&minscale_zoom13;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="10" fill="&cCityName;" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'suburb'</Filter>
		
		&maxscale_zoom14;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="13" fill="&cCityNameHighZoom;" halo-radius="1" wrap-width="0"/>
	</Rule>
</Style>
<Style name="village">
	<Rule>
		<Filter>[place] = 'village' or [place]='large_village'</Filter>
		
		&maxscale_zoom12;
		&minscale_zoom14;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="9" fill="&cCityName;" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'village' or [place]='large_village'</Filter>
		
		&maxscale_zoom15;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="12" fill="&cCityNameHighZoom;" halo-radius="1" wrap-width="0"/>
	</Rule>
</Style>
<Style name="hamlet">
	<Rule>
		<Filter>[place] = 'hamlet' or [place] = 'locality' or [place] = 'isolated_dwelling' or [place] = 'farm'</Filter>
		
		&maxscale_zoom14;
		&minscale_zoom15;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="8" fill="&cCityName;" halo-radius="1" wrap-width="0"/>
	</Rule>
	<Rule>
		<Filter>[place] = 'hamlet' or [place] = 'locality' or [place] = 'isolated_dwelling' or [place] = 'farm'</Filter>
		
		&maxscale_zoom16;
		
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="11" fill="&cCityNameHighZoom;" halo-radius="1" wrap-width="0"/>
	</Rule>
</Style>
