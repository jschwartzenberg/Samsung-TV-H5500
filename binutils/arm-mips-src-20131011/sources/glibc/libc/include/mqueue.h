#include <rt/mqueue.h>

#ifdef IS_IN_librt
hidden_proto (mq_timedsend)
hidden_proto (mq_timedreceive)
hidden_proto (mq_setattr)

extern __typeof (mq_timedsend_relative) __mq_timedsend_relative;
extern __typeof (mq_timedreceive_relative) __mq_timedreceive_relative;
#endif
