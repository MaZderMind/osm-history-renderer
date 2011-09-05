<Style name="pois-aeroway">
	<Rule>
      &maxscale_zoom10;
      &minscale_zoom10;
      <Filter>[aeroway]='aerodrome'</Filter>
      <PointSymbolizer file="&symbols;/png/16/transport/aerodrome.p.png" />
      <!--TextSymbolizer name="name" fontset_name="oblique-fonts" size="8" fill="#6692da" dy="-12" halo_radius="1" wrap_width="0"/-->
    </Rule>
    <Rule>
      &maxscale_zoom11;
      &minscale_zoom12;
      <Filter>[aeroway]='aerodrome'</Filter>
      <!--TextSymbolizer name="name" fontset_name="oblique-fonts" size="8" fill="#6692da" halo_radius="1" wrap_width="0"/-->
    </Rule>
    
    <Rule>
      &maxscale_zoom9;
      &minscale_zoom10;
      <Filter>[aeroway]='airport'</Filter>
      <PointSymbolizer file="&symbols;/png/16/transport/airport.p.png" />
      <!--TextSymbolizer name="name" fontset_name="bold-fonts" size="8" fill="#6692da" dy="-12" halo_radius="1" wrap_width="0"/-->
    </Rule>
    <Rule>
      &maxscale_zoom11;
      &minscale_zoom12;
      <Filter>[aeroway]='airport'</Filter>
      <!--TextSymbolizer name="name" fontset_name="bold-fonts" size="9" fill="#6692da" halo_radius="1" wrap_width="0"/-->
    </Rule>
    
    <Rule>
      &maxscale_zoom16;
      <Filter>[aeroway]='terminal'</Filter>
      <PointSymbolizer file="&symbols;/png/16/transport/airport_terminal.p.png" />
    </Rule>
    <Rule>
      &maxscale_zoom16;
      <Filter>[aeroway]='gate'</Filter>
      <PointSymbolizer file="&symbols;/png/16/transport/airport_gate.p.png" />
    </Rule>
    <Rule>
      &maxscale_zoom16;
      <Filter>[aeroway]='helipad'</Filter>
      <PointSymbolizer file="&symbols;/helipad.p.16.png" />
      <TextSymbolizer name="name" fontset_name="bold-fonts" size="8" fill="#6692da" dy="-10" halo_radius="1" wrap_width="0"/>
    </Rule>
</Style>
