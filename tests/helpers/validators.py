from testsuite.utils import matching

from helpers.models import User


def validate_user_reg(user: User, response):
    return response.json() == {
        'user': {
            'token': matching.RegexString(r'^[\w-]+\.[\w-]+\.[\w-]+$'),
            'username': user.username,
        },
    }

def validate_profile(user: User, response):
    return response.json() == {
        'user': {
            'username': user.username,
            'biography' user.biography,
            'display_name': user.display_name,
        },
    }