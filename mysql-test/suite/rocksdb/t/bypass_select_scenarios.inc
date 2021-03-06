--source ../include/bypass_create_table.inc

select @@rocksdb_select_bypass_policy;
select @@rocksdb_select_bypass_policy into @save_rocksdb_select_bypass_policy;
set global rocksdb_select_bypass_policy=2;
select @@rocksdb_select_bypass_policy;

SELECT variable_value into @executed_start
FROM information_schema.global_status WHERE
variable_name="rocksdb_select_bypass_executed";

--echo # Range query, binary key
let bypass_query=
SELECT /*+ bypass */ id1, id2, link_type, visibility, data, time, version
FROM link_table4 FORCE INDEX (id1_type)
WHERE link_type = 1 AND id1 = "a1\0\0\0\0\0\0\0\0\0\0\0\0\0\0" AND
      visibility = 1  AND time >= 0 AND time <= 4294967295
ORDER BY time DESC, id2 DESC LIMIT 0, 1000;
--source ../include/verify_bypass_query.inc

let bypass_query=
SELECT /*+ bypass */ id1, id2, link_type, visibility, data, time, version
FROM link_table4 FORCE INDEX (id1_type)
WHERE link_type = 1 AND id1 = "a1\0\0\0\0\0\0\0\0\0\0\0\0\0\0" AND
      visibility = 1  AND time >= 0 AND time <= 4294967295
ORDER BY time ASC, id2 ASC LIMIT 0, 1000;
--source ../include/verify_bypass_query.inc

--echo # Point query, flag = 0
let bypass_query=
SELECT /*+ bypass */ id, type, version, update_time, data
FROM node_table FORCE INDEX (PRIMARY)
WHERE id IN (1, 2, 3, 4, 5) AND type = 1
ORDER BY type, id;
--source ../include/verify_bypass_query.inc

let bypass_query=
SELECT /*+ bypass */ id, type, version, update_time, data
FROM node_table FORCE INDEX (PRIMARY)
WHERE id IN (1) AND type = 1;
--source ../include/verify_bypass_query.inc

let bypass_query=
SELECT /*+ bypass */ id, type, version, update_time, data
FROM node_table FORCE INDEX (PRIMARY)
WHERE id = 2 AND type = 1
ORDER BY type, id;
--source ../include/verify_bypass_query.inc

--echo # Point query, flag = 1
let bypass_query=
SELECT /*+ bypass */ id, type, row_created_time, is_deleted
FROM id_table FORCE INDEX (PRIMARY)
WHERE id IN (1, 2, 3, 4, 5)
ORDER BY id;
--source ../include/verify_bypass_query.inc

let bypass_query=
SELECT /*+ bypass */ id, type, row_created_time, is_deleted
FROM id_table FORCE INDEX (PRIMARY)
WHERE id IN (1);
--source ../include/verify_bypass_query.inc

let bypass_query=
SELECT /*+ bypass */ id, type, row_created_time, is_deleted
FROM id_table FORCE INDEX (PRIMARY)
WHERE id = 1;
--source ../include/verify_bypass_query.inc

--echo # Point query, flag = 2
let bypass_query=
SELECT /*+ bypass */ id, type, link_type, count, TIME, version
FROM count_table
WHERE id = 2 AND link_type = 1;
--source ../include/verify_bypass_query.inc

--echo # Bugs found when debugging real production scenarios
# pk_unpack_buf unassigned when unpack_pk=false and unpack_value=true
# PinnableSlice destroyed before send_row
let bypass_query=
SELECT /*+ bypass */ id1, id2, link_type, visibility, data, time, version
FROM link_table3 FORCE INDEX (id1_type)
WHERE link_type = 1 AND id1 = 1 AND visibility = 1
      AND time >= 0 AND time <= 4294967295
ORDER BY time DESC, id2 DESC LIMIT 0, 10000;
--source ../include/verify_bypass_query.inc

# Partial keys should not be unpacked
let bypass_query=
SELECT /*+ bypass */ data from link_table6 FORCE INDEX (id1_type) where id1=1;
--source ../include/verify_bypass_query.inc

# time >= ONLY crashes
let bypass_query=
SELECT /*+ bypass */ id1, id2, link_type, visibility, data, time, version
FROM link_table FORCE INDEX (id1_type)
WHERE link_type = 1 AND id1 = 1 AND visibility = 0 AND time >= 1551074583 AND
      time >= 0 AND time <= 4294967295
ORDER BY time DESC, id2 DESC LIMIT 0, 500;
--source ../include/verify_bypass_query.inc

let bypass_query=
SELECT /*+ bypass */ id1, id2, link_type, visibility, data, time, version
FROM link_table FORCE INDEX (id1_type)
WHERE link_type = 1 AND id1 = 1 AND visibility = 0 AND time >= 1551074583
ORDER BY time DESC, id2 DESC LIMIT 0, 500;
--source ../include/verify_bypass_query.inc

# IN expression multiplexing
let bypass_query=
SELECT /*+ bypass */ id1, id1_type, id2, id2_type FROM link_table
WHERE link_type=3 and id1 in (1, 2, 3)
ORDER BY link_type, id1, id2;
--source ../include/verify_bypass_query.inc

# IN expression multiplexing in middle
let bypass_query=
SELECT /*+ bypass */ id1, id1_type, id2, id2_type from link_table
WHERE link_type=3 and id1 IN (1, 2, 3) and id2=1
ORDER BY link_type, id1, id2;
--source ../include/verify_bypass_query.inc

# IN expression with multiple IN
let bypass_query=
SELECT /*+ bypass */ id1, id1_type, id2, id2_type from link_table
WHERE link_type=3 and id1 IN (1, 2, 3) and id2 IN (3, 4, 5)
ORDER BY link_type, id1, id2;
--source ../include/verify_bypass_query.inc

# Decoding varchar with extra padding spaces from index
let bypass_query=
SELECT /*+ bypass */ id1, id2, data, version, time
FROM link_table FORCE INDEX (id1_type)
WHERE id1=9 and link_type=5;
--source ../include/verify_bypass_query.inc

# Point query with duplicate items
let bypass_query=
SELECT /*+ bypass */ id, type, version, update_time, data
FROM node_table FORCE INDEX (PRIMARY)
WHERE id IN (5, 5, 5, 5, 5) AND type = 1;
--source ../include/verify_bypass_query.inc

# Point query with reverse order
let bypass_query=
SELECT /*+ bypass */ id, type, version, update_time, data
FROM node_table FORCE INDEX (PRIMARY)
WHERE id IN (5, 4, 3, 2, 1) AND type = 1;
--source ../include/verify_bypass_query.inc

# Point query with correct order
let bypass_query=
SELECT /*+ bypass */ id, type, version, update_time, data
FROM node_table FORCE INDEX (PRIMARY)
WHERE id IN (5, 4, 3, 2, 1) AND type = 1 ORDER BY id DESC;
--source ../include/verify_bypass_query.inc

# Range query with bloom filter
let bypass_query=
SELECT /*+ bypass */ id1, id2, data, time, version FROM link_table5
WHERE link_type=1 and id1=2 and id2 > 1;
let bypass_query=
SELECT /*+ bypass */ id1, id2, data, time, version FROM link_table5
WHERE link_type=1 and id1=2 and id2 >= 1;
--source ../include/verify_bypass_query.inc

--echo # Range query with PK key only
let bypass_query=
SELECT /*+ bypass */ id2 FROM link_table5
WHERE link_type=1 and id1=2 and id2=1;
--source ../include/verify_bypass_query.inc

let bypass_query=
SELECT /*+ bypass */ id2 FROM link_table5
WHERE link_type=1 and id1=2 and id2=1;
--source ../include/verify_bypass_query.inc

--echo # Range query with PK key only with filters
let bypass_query=
SELECT /*+ bypass */ id2 FROM link_table5
WHERE link_type=1 and id1=2 and id2 > 1;
--source ../include/verify_bypass_query.inc

let bypass_query=
SELECT /*+ bypass */ id2 FROM link_table5
WHERE link_type=1 and id1=2 and id2 >= 1;
--source ../include/verify_bypass_query.inc

# Prefix query returning large rows causing reallocation m_row_buf
# which lead to stale pointers in m_send_mapping.first
CREATE TABLE `link_table7` (
  `id1` bigint(20) unsigned NOT NULL DEFAULT '0' ,
  `id1_type` int(10) unsigned NOT NULL DEFAULT '0' ,
  `id2` bigint(20) unsigned NOT NULL DEFAULT '0' ,
  `id2_type` int(10) unsigned NOT NULL DEFAULT '0' ,
  `link_type` bigint(20) unsigned NOT NULL DEFAULT '0' ,
  `visibility` tinyint(3) NOT NULL DEFAULT '0' ,
  `data` varchar(255) COLLATE latin1_bin NOT NULL DEFAULT '' ,
  `time` int(10) unsigned NOT NULL DEFAULT '0' ,
  `version` bigint(20) unsigned NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`link_type` , `id1` , `id2`) COMMENT 'cf_link' ,
  KEY `id1_type` (`id1`,`link_type`,`visibility`,`time`,`id2`,`version`,`data`)
  COMMENT 'rev:cf_assoc_id1_type'
) ENGINE=ROCKSDB DEFAULT CHARSET=latin1 COLLATE=latin1_bin
ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=8;
insert into link_table7 values (9223372036854775807, 100, 9223372036854775801,
  100, 9223372036854775807, 0,
  'data1', 1234561, 9223372036854775801);
insert into link_table7 values (9223372036854775807, 100, 1223372036854775802,
  100, 9223372036854775807, 0,
  '0123456789012345678901234567890123456789012345678901234567890123456789'
  '0123456789012345678901234567890123456789012345678901234567890123456789'
  '0123456789012345678901234567890123456789012345678901234567890123456789'
  '0123456789012345678901234567890', 1234562, 9223372036854775802);
insert into link_table7 values (9223372036854775807, 100, 1223372036854775803,
  100, 9223372036854775807, 0,
  '0123456789012345678901234567890123456789012345678901234567890123456789'
  '0123456789012345678901234567890123456789012345678901234567890123456789'
  '0123456789012345678901234567890123456789012345678901234567890123456789'
  '0123456789012345678901234567890', 1234563, 9223372036854775803);

let bypass_query=
SELECT /*+ bypass */ id1, id2, link_type, visibility, data, time, version
FROM link_table7 FORCE INDEX (id1_type)
WHERE link_type = 9223372036854775807  and id1 = 9223372036854775807
and visibility = 0 ORDER BY time ASC, id2 ASC limit 0, 1000;
--source ../include/verify_bypass_query.inc

DROP TABLE link_table7;

# Incorrectly detecting 0x3 as unpacking in PK and returning 'data corruption'
CREATE TABLE `link_table8` (
  `id1` bigint(20) unsigned NOT NULL DEFAULT '0' ,
  `id1_type` int(10) unsigned NOT NULL DEFAULT '0' ,
  `id2` bigint(20) unsigned NOT NULL DEFAULT '0' ,
  `id2_type` int(10) unsigned NOT NULL DEFAULT '0' ,
  `link_type` bigint(20) unsigned NOT NULL DEFAULT '0' ,
  `visibility` tinyint(3) NOT NULL DEFAULT '0' ,
  `data` varchar(255) COLLATE latin1_bin NOT NULL DEFAULT '' ,
  `time` int(10) unsigned NOT NULL DEFAULT '0' ,
  `version` bigint(20) unsigned NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`link_type` , `id1` , `id2`) COMMENT 'cf_link' ,
  KEY `id1_type` (`id1`,`link_type`,`visibility`,`time`,`id2`,`version`,`data`)
  COMMENT 'rev:cf_assoc_id1_type'
) ENGINE=ROCKSDB DEFAULT CHARSET=latin1 COLLATE=latin1_bin
ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=8;
insert into link_table8 values (1, 0x03030303, 1, 0, 100, 0, 'data1', 0, 0);

let bypass_query=
SELECT /*+ bypass */ id1, id2, link_type, visibility, data, time, version
FROM link_table8 FORCE INDEX (PRIMARY) WHERE link_type = 100 and id1 = 1
ORDER BY id2 ASC limit 0, 1000;
--source ../include/verify_bypass_query.inc

DROP TABLE link_table8;

# KILL
--echo # Establish connection con1 (user=root)
connect (con1,localhost,root,,);

--echo # Switch to connection con1
connection con1;

let bypass_query=
SELECT /*+ bypass */ id1, id2, link_type, visibility, data, TIME, version
FROM link_table5 FORCE INDEX (PRIMARY)
WHERE link_type = 1 AND id1 = 1 AND visibility = 1
ORDER BY id2 ASC LIMIT 1;
--source ../include/verify_bypass_query.inc

--echo # Switch back to default connection
connection default;

select @@rocksdb_select_bypass_policy;
set global rocksdb_select_bypass_policy=@save_rocksdb_select_bypass_policy;
select @@rocksdb_select_bypass_policy;

SELECT variable_value into @executed_end
FROM information_schema.global_status WHERE
variable_name="rocksdb_select_bypass_executed";

SELECT @executed_end-@executed_start AS ROCKSDB_SELECT_BYPASS_EXECUTED;

DROP TABLE count_table;
DROP TABLE link_table;
DROP TABLE link_table2;
DROP TABLE id_table;
DROP TABLE node_table;
DROP TABLE link_table5;
DROP TABLE link_table5_rev;
DROP TABLE link_table3;
DROP TABLE link_table6;
DROP TABLE link_table4;
