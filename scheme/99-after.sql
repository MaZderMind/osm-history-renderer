ALTER TABLE hist_points ADD PRIMARY KEY (id, version);
CREATE INDEX hist_points_geom_and_time_index ON hist_points USING GIST (geom, valid_from, valid_to);

ALTER TABLE hist_lines ADD PRIMARY KEY (id, version, minor);
CREATE INDEX hist_lines_geom_and_time_index ON hist_points USING GIST (geom, valid_from, valid_to);

ALTER TABLE hist_areas ADD PRIMARY KEY (id, version, minor);
CREATE INDEX hist_areas_geom_and_time_index ON hist_points USING GIST (geom, valid_from, valid_to);

COMMIT;
