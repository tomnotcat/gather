TEMPLATE = subdirs
SUBDIRS += gtbase gtview gather backend
gtview.depends = gtbase
gather.depends = gtview
backend.depends = gtbase
