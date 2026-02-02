script_folder="/home/masaya/workspace/carla/test_package/build/gcc-11-x86_64-gnu17-release/generators"
echo "echo Restoring environment" > "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
for v in 
do
   is_defined="true"
   value=$(printenv $v) || is_defined="" || true
   if [ -n "$value" ] || [ -n "$is_defined" ]
   then
       echo export "$v='$value'" >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
   else
       echo unset $v >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
   fi
done
