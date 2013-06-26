TEMPLATE = subdirs
SUBDIRS += gtbase gtview gtsvce gather loader
gtview.depends = gtbase
gather.depends = gtview gtsvce
backend.depends = gtbase
