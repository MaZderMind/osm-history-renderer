<Style name="pois-place_of_worship">
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship' and [religion] = 'bahai'</Filter>
		<PointSymbolizer file="&symbols;/png/16/place_of_worship/bahai3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship' and [religion] = 'buddhist'</Filter>
		<PointSymbolizer file="&symbols;/png/16/place_of_worship/buddhist3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship' and [religion] = 'christian'</Filter>
		<PointSymbolizer file="&symbols;/png/16/place_of_worship/christian3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship' and [religion] = 'hindu'</Filter>
		<PointSymbolizer file="&symbols;/png/16/place_of_worship/hindu3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship' and [religion] = 'muslim'</Filter>
		<PointSymbolizer file="&symbols;png/16/place_of_worship/islamic3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship' and [religion] = 'jain'</Filter>
		<PointSymbolizer file="&symbols;/png/16/place_of_worship/jain3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship' and [religion] = 'jewish'</Filter>
		<PointSymbolizer file="&symbols;png/16/place_of_worship/jewish3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship' and [religion] = 'shinto'</Filter>
		<PointSymbolizer file="&symbols;/png/16/place_of_worship/shinto3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship' and [religion] = 'sikh'</Filter>
		<PointSymbolizer file="&symbols;/png/16/place_of_worship/sikh3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='place_of_worship'</Filter>
		<PointSymbolizer file="&symbols;/png/16/place_of_worship/unknown3.p.png" />
	</Rule>
	
	
	<Rule>
		<AnyFilter />
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="9" fill="&cPlaceOfWorshipText;" dy="9" halo-radius="1" wrap-width="0" />
	</Rule>
</Style>
