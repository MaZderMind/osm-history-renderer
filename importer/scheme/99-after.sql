ALTER TABLE hist_point ADD PRIMARY KEY (id, version);
CREATE INDEX hist_point_geom_and_time_index ON hist_point USING GIST (way, valid_from, valid_to);

ALTER TABLE hist_line ADD PRIMARY KEY (id, version, minor);
CREATE INDEX hist_line_geom_and_time_index ON hist_line USING GIST (way, valid_from, valid_to);

ALTER TABLE hist_polygon ADD PRIMARY KEY (id, version, minor);
CREATE INDEX hist_polygon_geom_and_time_index ON hist_polygon USING GIST (way, valid_from, valid_to);
