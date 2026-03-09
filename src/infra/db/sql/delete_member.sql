DELETE FROM chat.channel_members
WHERE channel_id = $1 AND user_id = $2;
-- todo надо через флаг is_deleted, точнее статус
