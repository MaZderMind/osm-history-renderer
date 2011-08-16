<Style name="pois-money">
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='atm'</Filter>
		<PointSymbolizer file="&symbols;/png/16/money/atm2.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='bank'</Filter>
		<PointSymbolizer file="&symbols;/png/16/money/bank2.p.png" />
	</Rule>
	
	
	<Rule>
		<AnyFilter />
		<TextSymbolizer name="[name]" fontset-name="book-fonts" size="9" fill="&cMoneyText;" dy="9" halo-radius="1" wrap-width="0" />
	</Rule>
</Style>
