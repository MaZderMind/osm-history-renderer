<Style name="pois-food">
	<!--
	  unused icons:
	    (all)
	-->
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='bar'</Filter>
		<PointSymbolizer file="&symbols;/png/16/food/bar.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='biergarten'</Filter>
		<PointSymbolizer file="&symbols;/png/16/food/biergarten.p.16.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>([amenity]='ice_cream') or ([amenity]='cafe' and [cuisine]='ice_cream')</Filter>
		<PointSymbolizer file="&symbols;/png/16/food/ice_cream.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='cafe' and [cuisine]!='ice_cream'</Filter>
		<PointSymbolizer file="&symbols;/png/16/food/cafe.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17; 
		<Filter>[amenity]='drinking_water'</Filter> 
		<PointSymbolizer file="&symbols;/png/16/food/drinkingtap.p.png" /> 
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='fast_food' and [cuisine]='pizza'</Filter>
		<PointSymbolizer file="&symbols;/png/16/food/fastfood_pizza.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='fast_food' and [cuisine]!='pizza'</Filter>
		<PointSymbolizer file="&symbols;/png/16/food/fastfood2.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='pub'</Filter>
		<PointSymbolizer file="&symbols;/png/16/food/pub.p.png" />
	</Rule>
	<Rule>
		&maxscale_zoom17;
		<Filter>[amenity]='restaurant'</Filter>
		<PointSymbolizer file="&symbols;/png/16/food/restaurant.p.png" />
	</Rule>
</Style>
