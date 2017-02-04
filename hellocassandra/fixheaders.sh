#!/bin/bash
sed -i "s/apache::thrift::protocol::TInputRecursionTracker/::apache::thrift::protocol::TInputRecursionTracker/" gen-cpp/Cassandra.cpp gen-cpp/cassandra_types.cpp
sed -i "s/apache::thrift::protocol::TOutputRecursionTracker/::apache::thrift::protocol::TOutputRecursionTracker/" gen-cpp/Cassandra.cpp gen-cpp/cassandra_types.cpp
