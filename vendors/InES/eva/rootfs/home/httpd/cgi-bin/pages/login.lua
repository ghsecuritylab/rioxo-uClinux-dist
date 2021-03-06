#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.cookie")

-- ------------------------------------------------------------------
-- Status page
-- ------------------------------------------------------------------
function show(t)
    local login = 0
 
    if(t.sent ~= nil) then
        if((t.in_web_user == t.web_user) and (t.in_web_pass == t.web_pass)) then
            hdoip.cookie.set(t, "username", t.web_user, {})        
            hdoip.cookie.set(t, "password", t.web_pass, {})       
            t.login = true 
        else 
            hdoip.html.AddError(t, label.p_lg_wrong)
        end
    end
 
    if (t.version_label == "rioxo") then
        page_name = label.page_name_rioxo
    elseif (t.version_label == "emcore") then
        page_name = label.page_name_emcore
    elseif (t.version_label == "black box") then
        page_name = label.page_name_black_box
    elseif (t.version_label == "riedel") then
        page_name = label.page_name_riedel
    else
        page_name = ""
    end

    hdoip.html.Header(t, page_name .. label.page_login, script_path)
    hdoip.html.Title(label.page_login)

    if(t.login) then
        hdoip.html.Text(label.p_lg_success)
    else 
        hdoip.html.TableHeader(2)
        hdoip.html.FormHeader(script_path, main_form)
    
        hdoip.html.Text(label.username);                        hdoip.html.TableInsElement(1);
        hdoip.html.FormText("in_web_user", "", 30, 0)           hdoip.html.TableInsElement(1);
        hdoip.html.Text(label.password);                        hdoip.html.TableInsElement(1);
        hdoip.html.FormPassword("in_web_pass", "", 30, 0)       hdoip.html.TableInsElement(1);
        hdoip.html.FormButton("submit", label.button_login)     hdoip.html.TableInsElement(2);
        hdoip.html.TableBottom()
    
        hdoip.html.FormEnd(t)
    end

    hdoip.html.Bottom(t)
end


