<Style name="pois-barrier">
	<Rule>
		&maxscale_zoom16;
		<Filter>[barrier]='bollard' or [barrier]='block'</Filter>
		<PointSymbolizer file="&symbols;/misc/bollard.png" allow-overlap="false" />
	</Rule>
	<Rule>
		&maxscale_zoom15;
		<Filter>[highway]='gate' or [barrier]='gate'</Filter>
		<PointSymbolizer file="&symbols;/misc/gate.png" allow-overlap="false"/>
	</Rule>
	<Rule> 
		&maxscale_zoom16; 
		<Filter>[barrier]='lift_gate'</Filter> 
		<PointSymbolizer file="&symbols;/misc/liftgate.png" allow-overlap="false"/> 
	</Rule> 
</Style>
