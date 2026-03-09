UPDATE chat.channel_members
SET role = chat.member_role_to_int('ADMIN')
WHERE channel_id = $1 AND role = chat.member_role_to_int('OWNER');
