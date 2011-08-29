<Style name="pois-education">
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='school'</Filter>
		<PointSymbolizer file="&symbols;/png/16/education/school.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom15;
		<Filter>[amenity]='college'</Filter>
		<PointSymbolizer file="&symbols;/png/16/education/college.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom15;
		<Filter>[amenity]='kindergarten'</Filter>
		<PointSymbolizer file="&symbols;/png/16/education/nursery3.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom15;
		<Filter>[amenity]='university'</Filter>
		<PointSymbolizer file="&symbols;/png/16/education/university.p.png" />
	</Rule>
	
	
	<Rule>
		<AlsoFilter />
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="9" fill="&cEducationText;" dy="9" halo-radius="1" wrap-width="0" />
	</Rule>
</Style>
