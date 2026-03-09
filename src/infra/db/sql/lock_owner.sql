SELECT user_id
FROM chat.channel_members
WHERE channel_id = $1 AND role = chat.member_role_to_int('OWNER')
FOR UPDATE;
