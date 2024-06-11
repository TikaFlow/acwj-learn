//
// Created by tika on 24-6-9.
//

#ifndef ACWJ_LEARN_ERRNO_H
#define ACWJ_LEARN_ERRNO_H

int * __errno_location(void);

#define errno (* __errno_location())

#endif //ACWJ_LEARN_ERRNO_H
