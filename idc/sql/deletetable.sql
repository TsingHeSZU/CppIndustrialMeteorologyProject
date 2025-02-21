-- 删除 T_ZHOBTMIND 表 2 小时之前的数据
delete from T_ZHOBTMIND where ddatetime < sysdate - 2/24;
-- 删除 T_ZHOBTMIND1 表 2 小时之前的数据
delete from T_ZHOBTMIND1 where ddatetime < sysdate - 2/24;
commit;
exit;