#ifndef HTSAVETOFILE_H
#define HTSAVETOFILE_H

extern HTStream * HTSaveToFile PARAMS((
        HTPresentation *        pres,
        HTParentAnchor *        anchor, /* Not used */
        HTStream *              sink));

extern HTStream * HTDumpToStdout PARAMS((
        HTPresentation *        pres,
        HTParentAnchor *        anchor, /* Not used */
        HTStream *              sink));

#endif /* HTSAVETOFILE_H */
