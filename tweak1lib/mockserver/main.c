/**
 * @file main.c
 * @ingroup tweak-compatibility-implementation-test
 * @brief part of tweak2 - tweak1 compatibility layer test suite.
 *
 * @copyright 2020-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @defgroup tweak-compatibility-implementation-test Test suite for tweak2 - tweak1 compatibility layer.
 */

#include <tweak.h>
#include <tweak2/appclient.h>
#include <tweak2/log.h>

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/signalfd.h>
#include <unistd.h>

static double tweak_get_def_val(const char* name) {
    (void)name;
    return 0.;
}

int main(int argc, const char** argv) {
    tweak_connect();

    tweak_add_groupbox("VIEW", "Standard rear view;Panoramic cos;Panoramic ellipse", 0);

    tweak_add_checkbox("pause", 0);
    tweak_add_checkbox("PROF", 0);
    tweak_add_checkbox("REFRESH", 0);
    tweak_add_checkbox("HMI_SEND", 0);
    tweak_add_checkbox("SHADER_HV", 0);
    tweak_add_spinbox("routine_id", 0, 65535, 0, 0);
    tweak_add_spinbox("routine_val", 0, 10, 0, 0);
 
    tweak_add_layout(300, 1, "/1");
    tweak_add_checkbox("patch BFS dump", 0);
    tweak_add_checkbox("patch current dump", 0);
    tweak_add_checkbox("hmi_exit_request", 0);
    tweak_add_checkbox("hv_refresh", 0);
    tweak_add_checkbox("restore_extrinsics", 0);
    tweak_add_checkbox("wiggle_extrinsics", 0);
    tweak_add_checkbox("update_intrinsics", 0);
    tweak_add_checkbox("calibrate", 0);
    tweak_add_checkbox("dump_calib_input", 0);
    tweak_add_checkbox("clear_local_pers", 0);
    tweak_add_checkbox("enable_blending", 1);

    tweak_add_layout(300, 1, "/2");

    tweak_add_slider("rear_2_origin", -4200.0, 4200.0, tweak_get_def_val("rear_2_origin"), 2);
    tweak_add_slider("pulse_delta", -10, 10, tweak_get_def_val("pulse_delta"), 0);
    tweak_add_slider("steering_ratio", 1.0, 15.0, tweak_get_def_val("steering_ratio"), 2);

    tweak_add_widget("/layout");
    tweak_add_layout(300, 0, "/layout");
    tweak_add_spinbox("ct_offset_x", 0.0, 2048, 0., 0);
    tweak_add_spinbox("ct_offset_y", 0.0, 2048, 0., 0);
    tweak_add_spinbox("ct_resolution_x", 2.0, 2048, 2., 0);
    tweak_add_spinbox("ct_resolution_y", 2.0, 2048, 2., 0);
    tweak_add_spinbox("cv_offset_x", 0.0, 2048, 0., 0);
    tweak_add_spinbox("cv_offset_y", 0.0, 2048, 0., 0);
    tweak_add_spinbox("cv_resolution_x", 2.0, 2048, 2., 0);
    tweak_add_spinbox("cv_resolution_y", 2.0, 2048, 2., 0);
    tweak_add_spinbox("tv_offset_x", 0, 2048, 0., 0);
    tweak_add_spinbox("tv_offset_y", 0.0, 2048, 0., 0);
    tweak_add_spinbox("tv_resolution_x", 4, 2048, 4., 0);
    tweak_add_spinbox("tv_resolution_y", 2.0, 2048, 2., 0);

    tweak_add_widget("/perf");
    tweak_add_layout(300, 0, "/perf");
    tweak_add_slider("vertical_angle_standard_view", -89.0, 89.0, tweak_get_def_val("vertical_angle_standard_view"), 1);
    tweak_add_slider("fov_x_standard_view", 70, 179, tweak_get_def_val("fov_x_standard_view"), 1);
    tweak_add_slider("ditortion_coefficient", 0.0, 1.0, tweak_get_def_val("ditortion_coefficient"), 2);
    tweak_add_slider("affine_offset", -3.0, 3.0, tweak_get_def_val("affine_offset"), 2);
    tweak_add_slider("guide_angle", -300.0, 300.0, tweak_get_def_val("guide_angle"), 1);

    tweak_add_slider("horizontal_distance_mm", 0.0, 10000.0, tweak_get_def_val("horizontal_distance_mm"), 0);
    tweak_add_slider("distance_near_mm", 0.0, 10000.0, tweak_get_def_val("distance_near_mm"), 0);
    tweak_add_slider("distance_center_mm", 0.0, 10000.0, tweak_get_def_val("distance_center_mm"), 0);
    tweak_add_slider("distance_far_mm", 0.0, 10000.0, tweak_get_def_val("distance_far_mm"), 0);
    tweak_add_slider("distance_start", 0.0, 10000.0, tweak_get_def_val("distance_start"), 0);
    tweak_add_slider("distance_end", 0.0, 10000.0, tweak_get_def_val("distance_end"), 0);
    tweak_add_slider("shadow_sz", 16.0, 64.0, tweak_get_def_val("shadow_sz"), 0);
    tweak_add_slider("shadow_max", 2.0, 64.0, tweak_get_def_val("shadow_max"), 0);
    tweak_add_slider("shadow_color", 2.0, 64.0, tweak_get_def_val("shadow_color"), 0);
 
    tweak_add_slider("color_rear_left_near_sz", 5.0, 40.0, tweak_get_def_val("color_rear_left_near_sz"), 0);
    tweak_add_slider("color_rear_left_far_sz", 5.0, 40.0, tweak_get_def_val("color_rear_left_far_sz"), 0);
    tweak_add_slider("color_rear_right_near_sz", 5.0, 40.0, tweak_get_def_val("color_rear_right_near_sz"), 0);
    tweak_add_slider("color_rear_right_far_sz", 5.0, 40.0, tweak_get_def_val("color_rear_right_far_sz"), 0);
    tweak_add_slider("color_rear_hor_near_sz", 5.0, 40.0, tweak_get_def_val("color_rear_hor_near_sz"), 0);
    tweak_add_slider("color_rear_hor_center_sz", 5.0, 40.0, tweak_get_def_val("color_rear_hor_center_sz"), 0);
    tweak_add_slider("color_rear_hor_far_sz", 5.0, 40.0, tweak_get_def_val("color_rear_hor_far_sz"), 0);

    tweak_add_slider("h_r", 0.0, 255.0, tweak_get_def_val("h_r"), 0);
    tweak_add_slider("h_g", 0.0, 255.0, tweak_get_def_val("h_g"), 0);
    tweak_add_slider("h_b", 0.0, 255.0, tweak_get_def_val("h_b"), 0);
    tweak_add_slider("l_r", 0.0, 255.0, tweak_get_def_val("l_r"), 0);
    tweak_add_slider("l_g", 0.0, 255.0, tweak_get_def_val("l_g"), 0);
    tweak_add_slider("l_b", 0.0, 255.0, tweak_get_def_val("l_b"), 0);

    tweak_add_layout(300, 1, "/cross_view");
    tweak_add_slider("hv_update_dist", 0.017, 0.85, tweak_get_def_val("hv_update_dist"), 2);
    tweak_add_slider("fov_x_cross_view", 70, 200, tweak_get_def_val("fov_x_cross_view"), 1);
    tweak_add_slider("vertical_angle_cross_view", -90.0, 90.0, tweak_get_def_val("vertical_angle_cross_view"), 1);
    tweak_add_slider("fov_y", 70, 200, tweak_get_def_val("fov_y"), 1);
    tweak_add_slider("curve_scale", 1.0, 8.0, tweak_get_def_val("curve_scale"), 2);
    tweak_add_slider("horizont_shift", -1.0, 1.0, tweak_get_def_val("horizont_shift"), 2);
    tweak_add_slider("z_distortion", 0.0, 1.0, tweak_get_def_val("z_distortion"), 2);

 
    tweak_add_widget("/state_machine");
    tweak_add_layout(150, 0, "/state_machine");

    tweak_add_spinbox("sel_view_type_vpark", 0, 9, 5, 0);

    tweak_add_spinbox("reset_HS7_DONNEES_VSM_LENTES", 0, 255, 0, 0);
    tweak_add_spinbox("set_angle_volant", 0, 255, 0, 0);
    tweak_add_spinbox("set_dmde_audio", 0, 255, 0, 0);
    tweak_add_spinbox("set_etat_aas_ar", 0, 255, 0, 0);
    tweak_add_spinbox("set_etat_cpo_coffre", 0, 255, 0, 0);
    tweak_add_spinbox("set_etat_dmd_activ_vpark", 0, 255, 0, 0);
    tweak_add_spinbox("set_etat_gmp", 0, 255, 0, 0);
    tweak_add_spinbox("set_etat_ma", 0, 255, 0, 0);
    tweak_add_spinbox("set_etat_princip_sev", 0, 255, 0, 0);
    tweak_add_spinbox("set_localisation_aas", 0, 255, 0, 0);
    tweak_add_spinbox("set_mode_config_vhl", 0, 255, 0, 0);
    tweak_add_spinbox("set_pid_dist_armd", 0, 255, 0, 0);
    tweak_add_spinbox("set_pid_dist_armg", 0, 255, 0, 0);
    tweak_add_spinbox("set_pid_dmde_affichage", 0, 255, 0, 0);
    tweak_add_spinbox("set_p_info_acpk_menu", 0, 255, 0, 0);
    tweak_add_spinbox("set_remorque_presente", 0, 255, 0, 0);
    tweak_add_spinbox("set_repart_aas", 0, 255, 0, 0);
    tweak_add_spinbox("set_sel_ov_vpark", 0, 255, 0, 0);
    tweak_add_spinbox("set_sel_view_type_vpark", 0, 255, 0, 0);
    tweak_add_spinbox("set_sel_view_vpark", 0, 255, 0, 0);
    tweak_add_spinbox("set_sel_vpark_brightness", 0, 255, 0, 0);
    tweak_add_spinbox("set_sel_vpark_contrast", 0, 255, 0, 0);
    tweak_add_spinbox("set_sens_roulage", 0, 255, 0, 0);
    tweak_add_spinbox("set_vitesse_longitudionale", 0, 255, 0, 0);
    tweak_add_spinbox("update_puls_cnt_wheel", 0, 255, 0, 0);
    tweak_add_spinbox("push_view_t", 0, 255, 0, 0);
    tweak_add_slider("z_angle", 0.0, 80.0, 0.0, 1);

    sigset_t mask;
    int sfd;
    struct signalfd_siginfo fdsi;

    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);

    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1) {
        return EXIT_FAILURE;
    }

    ssize_t s = read(sfd, &fdsi, sizeof(fdsi));
    if (s == sizeof(fdsi)) {
        switch(fdsi.ssi_signo) {
        case SIGHUP:
            printf("Got SIGHUP\n");
            break;
        case SIGINT:
            printf("Got SIGINT\n");
            break;
        case SIGQUIT:
            printf("Got SIGQUIT\n");
            break;
        case SIGTERM:
            printf("Got SIGTERM\n");
            break;
        default:
            printf("Read unexpected signal :%d\n", fdsi.ssi_signo);
            break;
        }
    } else {
        printf("Invalid number of bytes being read\n");
    }

    tweak_close();
    return 0;
}