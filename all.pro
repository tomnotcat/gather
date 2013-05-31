TEMPLATE = subdirs
SUBDIRS += gtbase gtview gather loader
gtview.depends = gtbase
gather.depends = gtview
backend.depends = gtbase
