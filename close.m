function close()
%CLOSE Close the tdk tactor interface.
tactor('shutdown');
clear tactor;
end