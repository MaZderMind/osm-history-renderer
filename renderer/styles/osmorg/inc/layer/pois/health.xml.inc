<Style name="pois-health">
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='dentist'</Filter>
		<PointSymbolizer file="&symbols;/png/16/health/dentist.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom15;
		<Filter>[amenity]='hospital'</Filter>
		<PointSymbolizer file="&symbols;/png/16/health/hospital.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='pharmacy'</Filter>
		<PointSymbolizer file="&symbols;/png/16/health/pharmacy.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='doctors'</Filter>
		<PointSymbolizer file="&symbols;/png/16/health/doctors2.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='veterinary'</Filter>
		<PointSymbolizer file="&symbols;/png/16/health/veterinary.p.png" />
	</Rule>
	
	
	<Rule>
		<AnyFilter />
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="9" fill="&cHealthText;" dy="9" halo-radius="1" wrap-width="0" />
	</Rule>
</Style>
