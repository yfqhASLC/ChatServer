


#定义了一个SRC_LIST变量，包含了该目录下(不包含子目录下的)所有的源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LSIT)
aux_source_directory(./model MODEL_LSIT)
aux_source_directory(./redis REDIS_LSIT)   


#指定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LSIT} ${MODEL_LSIT} ${REDIS_LSIT})


#指定可执行文件链接时依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)

