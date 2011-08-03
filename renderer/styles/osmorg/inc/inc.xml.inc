<!ENTITY % settings SYSTEM "settings.xml.inc">
%settings;

<!ENTITY % colors SYSTEM "colors.xml.inc">
%colors;

<!ENTITY % layers SYSTEM "layers.xml.inc">
%layers;

<!ENTITY % scales SYSTEM "scales.xml.inc">
%scales;

<!ENTITY datasource-settings SYSTEM "datasource-settings.xml.inc">
<!ENTITY fontset-settings SYSTEM "fontset-settings.xml.inc">

<!ENTITY srsmercator "+proj=merc +datum=WGS84 +over">
<!ENTITY srs900913 "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over">
<!ENTITY srs4326 "+init=epsg:4326">

<!ENTITY date-where "'{DATE}' BETWEEN valid_from AND COALESCE(valid_to, NOW())">
