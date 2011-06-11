function Filter() {
    var cb;
}

Filter.prototype.on = function(cb) {
    this.cb = cb;
}

// TODO extend argument model
Filter.prototype.tag = function(tag) {
    var filter = new Filter();
    
    if(typeof tag == 'string') {
        this.cb = function(node) {
            if(node.tags && node.tags[tag]) {
                filter.raise(node);
            }
        }
    }
    else if(typeof tag == 'object') {
        this.cb = function(node) {
            if(!node.tags.length == 0) {
                return;
            }
            for(k in tag) {
                if(node.tags[k]) {
                    var v = tag[k];
                    if(typeof v == 'string' && node.tags[k] == v) {
                        return filter.raise(node);
                    }
                    else if(v.length) {
                        for(i=0, l=v.length; i<l; i++) {
                            if(node.tags[k] == v[i])  {
                                return filter.raise(node);
                            }
                        }
                    }
                }
            }
        }
    }
    return filter;
}

Filter.prototype.raise = function(node) {
    this.cb(node);
}

var importer = {
    
    node_filters: [], 
    
    node: function() {
        var filter = new Filter();
        this.node_filters.push(filter);
        return filter;
    }
}

Osmium.Callbacks.node = function() {
    var f = importer.node_filters;
    for(var i=0, l=f.length; i<l; i++) {
        f[i].raise(this);
    }
}

