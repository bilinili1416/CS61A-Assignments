class Link():
    empty = ()
    def __init__(self, first, rest=empty):
        self.first = first
        self.rest = rest

    def __repr__(self):
        # implement the __repr__ method
        if self.rest is Link.empty:
            return 'Link({})'.format(repr(self.first))
        return 'Link({}, {})'.format(repr(self.first), repr(self.rest))
        
def add(s, v):
    li = s
    while li.rest != Link.empty:
        li = li.rest
    li.rest = Link(v)
    return s
    
s = Link(1, Link(2, Link(3)))
s.first = 5

# print(s.rest.rest.rest.rest.rest.first)

print(add(s, 6))