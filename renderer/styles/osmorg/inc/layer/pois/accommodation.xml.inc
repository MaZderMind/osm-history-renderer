<Style name="pois-accommodation">
	<Rule>
		&maxscale_zoom13;
		<Filter>[tourism]='alpine_hut'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/alpinehut.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[tourism]='bed_and_breakfast'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/bed_and_breakfast.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[tourism]='camp_site'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/camping.n.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[tourism]='caravan_site'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/caravan_park.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[tourism]='chalet'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/chalet.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[tourism]='hostel' or [tourism]='youth_hostel'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/hostel.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[tourism]='guest_house'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/hotel.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[tourism]='hotel'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/hotel2.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[tourism]='motel'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/motel.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom16;
		<Filter>[amenity]='shelter'</Filter>
		<PointSymbolizer file="&symbols;/png/16/accommodation/shelter2.p.png" />
	</Rule>
	
	
	<Rule>
		<AlsoFilter />
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="9" fill="&cAccommodationText;" dy="9" halo-radius="1" wrap-width="0" />
	</Rule>
</Style>
